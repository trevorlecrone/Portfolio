#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

/**
* Image
**/
Image::Image(int width_, int height_) {

    assert(width_ > 0);
    assert(height_ > 0);

    width = width_;
    height = height_;
    num_pixels = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;

    data.raw = new uint8_t[num_pixels * 4];
    int b = 0; //which byte to write to
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            data.raw[b++] = 0;
            data.raw[b++] = 0;
            data.raw[b++] = 0;
            data.raw[b++] = 0;
        }
    }

    assert(data.raw != NULL);
}

double pi = atan(1) * 4;

Image::Image(const Image& src) {

    width = src.width;
    height = src.height;
    num_pixels = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;

    data.raw = new uint8_t[num_pixels * 4];

    //memcpy(data.raw, src.data.raw, num_pixels);
    *data.raw = *src.data.raw;
}

Image::Image(char* fname) {

    int numComponents; //(e.g., Y, YA, RGB, or RGBA)
    data.raw = stbi_load(fname, &width, &height, &numComponents, 4);

    if (data.raw == NULL) {
        printf("Error loading image: %s", fname);
        exit(-1);
    }


    num_pixels = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;

}

Image::~Image() {
    delete data.raw;
    data.raw = NULL;
}

void Image::Write(char* fname) {

    int lastc = strlen(fname);

    switch (fname[lastc - 1]) {
    case 'g': //jpeg (or jpg) or png
        if (fname[lastc - 2] == 'p' || fname[lastc - 2] == 'e') //jpeg or jpg
            stbi_write_jpg(fname, width, height, 4, data.raw, 95);  //95% jpeg quality
        else //png
            stbi_write_png(fname, width, height, 4, data.raw, width * 4);
        break;
    case 'a': //tga (targa)
        stbi_write_tga(fname, width, height, 4, data.raw);
        break;
    case 'p': //bmp
    default:
        stbi_write_bmp(fname, width, height, 4, data.raw);
    }
}

int clampInt(int val) {
    return val > 255 ? 255 : val < 0 ? 0 : val;
}

void Image::AddNoise(double factor)
{
    int x, y;
    int new_r, new_g, new_b;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            //generate a random pixel value and mmultiply by our noise factor
            Pixel rand_p = PixelRandom();
            rand_p = rand_p * factor;

            //multiply the pixel in the image by 1 - the factor
            Pixel p = GetPixel(x, y);
            p = (p * (1.0 - factor));
            int add_or_sub = rand() % 2;
            if (add_or_sub == 0) {
                new_r = clampInt(p.r + rand_p.r);
                new_g = clampInt(p.g + rand_p.g);
                new_b = clampInt(p.b + rand_p.b);
            }
            else {
                new_r = clampInt(p.r - rand_p.r);
                new_g = clampInt(p.g - rand_p.g);
                new_b = clampInt(p.b - rand_p.b);
            }
            //combine the two above pixels into our result
            Pixel result = Pixel(new_r, new_g, new_b);
            SetPixel(x, y, result);
        }
    }
}



void Image::Brighten(double factor)
{
    int x, y;
    for (x = 0; x < Width(); x++)
    {
        for (y = 0; y < Height(); y++)
        {
            Pixel p = GetPixel(x, y);
            Pixel scaled_p = p*factor;
            GetPixel(x, y) = scaled_p;
        }
    }
}


void Image::ChangeContrast(double factor)
{
    factor = factor < 0 ? 0 : factor;
    double avg_lum = 0.0;
    int x, y;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            avg_lum += p.Luminance();
        }
    }
    avg_lum = avg_lum / NumPixels();
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            double new_r = round((factor * (p.r - avg_lum)) + avg_lum);
            double new_g = round((factor * (p.g - avg_lum)) + avg_lum);
            double new_b = round((factor * (p.b - avg_lum)) + avg_lum);
            new_r = clampInt(new_r);
            new_g = clampInt(new_g);
            new_b = clampInt(new_b);
            Pixel result = Pixel((int)new_r, (int)new_g, (int)new_b);
            SetPixel(x, y, result);
        }
    }
}


void Image::ChangeSaturation(double factor)
{

    int x, y;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            //preserve pixel luminance
            double lum = p.Luminance();
            Pixel grey = Pixel(lum, lum, lum);
            Pixel result = PixelLerp(grey, p, factor);
            SetPixel(x, y, result);
        }
    }
}


Image* Image::Crop(int x, int y, int w, int h)
{
    //clamp values so passing negatives and going out of bounds do not seg fault
    x = (x < 0) ? 0 : (x > Width()) ? Width() : x;
    y = (y < 0) ? 0 : (y > Height()) ? Height() : y;
    int start_y = y;
    int start_x = x;
    int r_edge = ((x + w) < 0) ? 0 : ((x + w) > Width()) ? Width() : (x + w);
    int t_edge = ((y + h) < 0) ? 0 : ((y + h) > Height()) ? Height() : (y + h);

    Image* output = new Image(w, h);
    //output coords
    int o_x = 0;
    int o_y = 0;
    for (y = start_y; y < t_edge; y++) {
        o_x = 0;
        for (x = start_x; x < r_edge; x++) {
            if (!ValidCoord(x, y)) printf("x: %d, y: %d\n", x, y);
            if (!output->ValidCoord(o_x, o_y)) printf("invalid output x: %d, y: %d\n", o_x, o_y);
            Pixel p = GetPixel(x, y);
            output->SetPixel(o_x, o_y, p);
            o_x++;
        }
        o_y++;
    }
    return output;
}


void Image::ExtractChannel(int channel)
{
    double e_red = 0.0;
    double e_green = 0.0;
    double e_blue = 0.0;
    if (channel == 1) {
        e_red = 1.0;
    }
    else if (channel == 2) {
        e_green = 1.0;
    }
    else if (channel == 3) {
        e_blue = 1.0;
    }
    int x, y;
    double new_r;
    double new_g;
    double new_b;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            new_r = p.r * e_red;
            new_g = p.g * e_green;
            new_b = p.b * e_blue;
            Pixel result = Pixel(new_r, new_g, new_b);
            SetPixel(x, y, result);
        }
    }
}


void Image::Quantize(int nbits)
{
    int x, y;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            Pixel result = PixelQuant(p, nbits);
            SetPixel(x, y, result);
        }
    }
}

void Image::RandomDither(int nbits)
{
    //add our randomm noise before we begin, only enough that we could bump the pixel 
    //up or down one "bin"
    double bins = pow(2, nbits);
    AddNoise(1.0 / bins);
    int x, y;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            Pixel result = PixelQuant(p, nbits);
            SetPixel(x, y, result);
        }
    }
}


static int Bayer4[4][4] =
{
    { 15,  7, 13,  5 },
    { 3, 11,  1,  9 },
    { 12,  4, 14,  6 },
    { 0,  8,  2, 10 }
};


void Image::OrderedDither(int nbits)
{
    double bins = pow(2, nbits);
    int interval = round(255.0 / bins);
    int x, y;
    int i, j;
    int new_r, new_g, new_b;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            i = x % 4;
            j = y % 4;
            Pixel p = GetPixel(x, y);
            new_r = clampInt(p.r - round((interval * (Bayer4[i][j] / 16.0))));
            new_g = clampInt(p.g - round((interval * (Bayer4[i][j] / 16.0))));
            new_b = clampInt(p.b - round((interval * (Bayer4[i][j] / 16.0))));
            Pixel result = Pixel(new_r, new_g, new_b);
            result = PixelQuant(result, nbits);
            SetPixel(x, y, result);
        }
    }
}

/* Error-diffusion parameters */
const double
ALPHA = 7.0 / 16.0,
BETA = 3.0 / 16.0,
GAMMA = 5.0 / 16.0,
DELTA = 1.0 / 16.0;

void Image::FloydSteinbergDither(int nbits)
{
    double bins = pow(2, nbits);
    int x, y;
    Pixel alpha;
    Pixel beta;
    Pixel gamma;
    Pixel delta;
    int new_r, new_g, new_b;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            Pixel quant_p = PixelQuant(p, nbits);
            int d_r = p.r - quant_p.r;
            int d_g = p.g - quant_p.g;
            int d_b = p.b - quant_p.b;

            bool canAlpha = ValidCoord(x + 1, y);
            bool canBeta = ValidCoord(x - 1, y + 1);
            bool canGamma = ValidCoord(x, y + 1);
            bool canDelta = ValidCoord(x + 1, y + 1);

            if (canAlpha) {
                alpha = GetPixel(x + 1, y);
                new_r = clampInt(alpha.r + (d_r * ALPHA));
                new_g = clampInt(alpha.g + (d_g * ALPHA));
                new_b = clampInt(alpha.b + (d_b * ALPHA));
                alpha = Pixel(new_r, new_g, new_b);
                SetPixel(x + 1, y, alpha);
            }
            if (canBeta) {
                beta = GetPixel(x - 1, y + 1);
                new_r = clampInt(beta.r + (d_r * BETA));
                new_g = clampInt(beta.g + (d_g * BETA));
                new_b = clampInt(beta.b + (d_b * BETA));
                beta = Pixel(new_r, new_g, new_b);
                SetPixel(x - 1, y + 1, beta);
            }
            if (canGamma) {
                gamma = GetPixel(x, y + 1);
                new_r = clampInt(gamma.r + (d_r * GAMMA));
                new_g = clampInt(gamma.g + (d_g * GAMMA));
                new_b = clampInt(gamma.b + (d_b * GAMMA));
                gamma = Pixel(new_r, new_g, new_b);
                SetPixel(x, y + 1, gamma);
            }
            if (canDelta) {
                delta = GetPixel(x + 1, y + 1);
                new_r = clampInt(delta.r + (d_r * DELTA));
                new_g = clampInt(delta.g + (d_g * DELTA));
                new_b = clampInt(delta.b + (d_b * DELTA));
                delta = Pixel(new_r, new_g, new_b);
                SetPixel(x + 1, y + 1, delta);
            }

            SetPixel(x, y, quant_p);
        }
    }
}

float** CreateGaussian(double blur, int purpose) {
    float** kernel;
    if (blur == 0) {
        kernel = new float *[1];
        kernel[0] = new float[1];
        kernel[0][0] = 1.0;
        return kernel;
    }
    //for our blur tool
    if (purpose == 0) {
        kernel = new float *[5];
        kernel[0] = new float[5];
        kernel[1] = new float[5];
        kernel[2] = new float[5];
        kernel[3] = new float[5];
        kernel[4] = new float[5];
        double sigma = blur;
        double avg = 2.5;
        double kernel_sum = 0.0;
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                //code recycled from my 3081 project where whe implemented this function
                kernel[x][y] = exp(-0.5 * (pow((x - avg) / blur, 2.0) + pow((y - avg) / blur, 2.0))) / (2 * pi * blur * blur);
                kernel_sum += kernel[x][y];
            }
        }
        for (int x = 0; x < 5; ++x) {
            for (int y = 0; y < 5; ++y) {
                kernel[x][y] /= kernel_sum;
            }
        }
        return kernel;
    }
    //for our sampling
    else {
        kernel = new float *[3];
        kernel[0] = new float[3];
        kernel[1] = new float[3];
        kernel[2] = new float[3];
        double sigma = blur;
        double avg = 1.5;
        double kernel_sum = 0.0;
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                kernel[x][y] = exp(-0.5 * (pow((x - avg) / blur, 2.0) + pow((y - avg) / blur, 2.0))) / (2 * pi * blur * blur);
                kernel_sum += kernel[x][y];
            }
        }
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                kernel[x][y] /= kernel_sum;
            }
        }
        return kernel;
    }
}


void Image::Blur(int n)
{
    int num_applications = ceil((float)n / 2);
    Image* copy = new Image(*this);
    float** kernel = CreateGaussian(1 + (n % 2), 0);
    int x, y;
    int size = 5;
    int x_coord, y_coord;
    int e_x, e_y;
    float new_r;
    float new_g;
    float new_b;
    for (int z = 0; z < num_applications; z++) {
        int i, j;
        for (j = 0; j < Height(); j++) {
            for (i = 0; i < Width(); i++) {
                Pixel p = GetPixel(i, j);
                copy->SetPixel(i, j, p);
            }
        }
        for (y = 0; y < Height(); y++) {
            for (x = 0; x < Width(); x++) {
                new_r = 0.0;
                new_g = 0.0;
                new_b = 0.0;
                Pixel p = GetPixel(x, y);
                for (e_y = 0; e_y < size; e_y++) {
                    for (e_x = 0; e_x < size; e_x++) {
                        x_coord = x + e_x - 2;
                        y_coord = y + e_y - 2;
                        if ((x_coord > 0) && (x_coord < Width()) && (y_coord > 0) && (y_coord < Height()));
                        else {
                            if (x_coord < 0 || x_coord >= Width()) {
                                x_coord = x - (e_x - 2);
                            }
                            if (y_coord < 0 || y_coord >= Height()) {
                                y_coord = y - (e_y - 2);
                            }
                        }
                        Pixel p_1 = copy->GetPixel(x_coord, y_coord);
                        float red = p_1.r;
                        float green = p_1.g;
                        float blue = p_1.b;
                        new_r = new_r + (red * kernel[e_x][e_y]);
                        new_g = new_g + (green * kernel[e_x][e_y]);
                        new_b = new_b + (blue * kernel[e_x][e_y]);
                    }
                }
                Pixel result = Pixel((int)new_r, (int)new_g, (int)new_b);
                SetPixel(x, y, result);
            }
        }
    }
}

void Image::Sharpen(int n)
{
    Image* copy = new Image(*this);
    int x, y;
    int i, j;
    for (j = 0; j < Height(); j++) {
        for (i = 0; i < Width(); i++) {
            Pixel p = GetPixel(i, j);
            copy->SetPixel(i, j, p);
        }
    }
    copy->Blur(n);
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            Pixel p = GetPixel(x, y);
            Pixel blurred = copy->GetPixel(x, y);
            Pixel result = PixelLerp(blurred, p, 2);
            SetPixel(x, y, result);
        }
    }
}


void Image::EdgeDetect()
{
    //uses Sobel operations to get average x and y gradients.
    //also recycled from my 3081 project.
    static int t_b_edge[3][3] =
    {
        { 1, 0, -1 },
        { 2, 0, -2 },
        { 1, 0, -1 },
    };
    static int l_r_edge[3][3] =
    {
        { 1, 2, 1 },
        { 0, 0, 0 },
        { -1, -2, -1 },
    };
    Image* copy = new Image(*this);
    int i, j;
    for (j = 0; j < Height(); j++) {
        for (i = 0; i < Width(); i++) {
            Pixel p = GetPixel(i, j);
            copy->SetPixel(i, j, p);
        }
    }
    int x, y;
    int e_x, e_y;
    int x_coord, y_coord;
    int h_grad, v_grad;
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            h_grad = 0;
            v_grad = 0;
            for (e_y = 0; e_y < 3; e_y++) {
                for (e_x = 0; e_x < 3; e_x++) {
                    x_coord = x + e_x - 1;
                    y_coord = y + e_y - 1;
                    if ((x_coord >= 0) && (x_coord < Width()) && (y_coord >= 0) && (y_coord < Height())) {
                        Pixel p = copy->GetPixel(x_coord, y_coord);
                        h_grad += p.Luminance() * l_r_edge[e_x][e_y];
                        v_grad += p.Luminance() * t_b_edge[e_x][e_y];
                    }
                    else {
                        if (x_coord < 0 || x_coord >= Width()) {
                            x_coord = x - (e_x - 1);
                        }
                        if (y_coord < 0 || y_coord >= Height()) {
                            y_coord = y - (e_y - 1);
                        }
                        Pixel p = copy->GetPixel(x_coord, y_coord);
                        h_grad += p.Luminance() * l_r_edge[e_x][e_y];
                        v_grad += p.Luminance() * t_b_edge[e_x][e_y];
                    }
                }
            }

            int grad_avg = sqrt((h_grad * h_grad) + (v_grad*v_grad));
            Pixel result = Pixel(grad_avg, grad_avg, grad_avg);
            SetPixel(x, y, result);
        }
    }
}

Image* Image::Scale(double sx, double sy)
{
    Image* newImg = new Image(Width() * sx, Height() * sy);
    double inverse_x = 1 / sx;
    double inverse_y = 1 / sy;
    int x, y;
    for (y = 0; y < newImg->Height(); y++) {
        for (x = 0; x < newImg->Width(); x++) {
            double u = x * inverse_x;
            double v = y * inverse_y;
            Pixel p = Sample(u, v);
            newImg->SetPixel(x, y, p);
        }
    }
    return newImg;
}

Image* Image::Rotate(double angle)
{
    int is_90 = floor(angle / (pi / 2.0));
    int new_w, new_h;
    if (is_90 % 2 == 1) {
        double dim_angle = angle - pi / 2;
        new_h = (Height() * sin(dim_angle)) + (Width() * cos(dim_angle));
        new_w = (Height() * cos(dim_angle)) + (Width() * sin(dim_angle));
    }
    else {
        new_w = (Width() * cos(angle)) + (Height() * sin(angle));
        new_h = (Width() * sin(angle)) + (Height() * cos(angle));
    }
    double center_x = Width() / 2;
    double center_y = Height() / 2;
    double ratio_x = (float) new_w / Width();
    double ratio_y = (float) new_h / Height();

    Image* newImg = new Image(new_w, new_h);
    int x, y;

    for (y = 0; y < new_h; y++) {
        for (x = 0; x < new_w; x++) {
            double x_c = (x - (new_w / 2));
            double y_c = (y - (new_h / 2));
            //if (x == (new_w / 2) && y == (new_h / 2)) {
            //    printf("x: %d, x_rat: %f\n x_x_rat: %f")
            //}
            double u = ((x_c * cos(-angle)) - (y_c * sin(-angle)));
            double v = ((x_c * sin(-angle)) + (y_c * cos(-angle)));
            u = (u + center_x);
            v = (v + center_y);
            Pixel p = Sample(u, v);
            newImg->SetPixel(x, y, p);
        }
    }
    return newImg;
}

void Image::Fun()
{
    Image* copy = new Image(*this);
    int i, j;
    for (j = 0; j < Height(); j++) {
        for (i = 0; i < Width(); i++) {
            Pixel p = GetPixel(i, j);
            copy->SetPixel(i, j, p);
        }
    }
    int x, y;
    int center_y = Height() / 2;
    int center_x = Width() / 2;
    float dist_to_corner = (center_x*center_x) + (center_y*center_y);
    for (y = 0; y < Height(); y++) {
        for (x = 0; x < Width(); x++) {
            double u = x + (5 * cos(y / (pi / 20)));
            double v = y + (5 * sin(x / (pi / 20)));
            if (!ValidCoord(int(u), int(v))) {
                u = x - (5 * cos(y / (pi / 20)));
                v = y - (5 * sin(x / (pi / 20)));
            }
            Pixel p = copy->Sample(u, v);
            SetPixel(x, y, p);
        }
    }



}

/**
* Image Sample
**/
void Image::SetSamplingMethod(int method)
{
    assert((method >= 0) && (method < IMAGE_N_SAMPLING_METHODS));
    sampling_method = method;
}


Pixel Image::Sample(double u, double v) {
    if (sampling_method == IMAGE_SAMPLING_POINT) {
        int source_x = u;
        int source_y = v;
        if (ValidCoord(source_x, source_y)) {
            return GetPixel(source_x, source_y);
        }
        else {
            return Pixel(0, 0, 0);
        }
    }
    if (sampling_method == IMAGE_SAMPLING_BILINEAR) {
        int l_x = floor(u);
        int t_y = floor(v);
        int r_x = l_x + 1;
        int b_y = t_y + 1;
        int x;
        int y;
        Pixel tl = Pixel();
        Pixel tr = Pixel();
        Pixel bl = Pixel();
        Pixel br = Pixel();
        double num_sampled = 0.0;
        double total_dist = 0.0;
        double l_dist = u - l_x;
        double r_dist = 1.0 - l_dist;
        double t_dist = v - t_y;
        double b_dist = 1.0 - t_dist;

        double t_l_w = r_dist*b_dist;
        double t_r_w = l_dist*b_dist;
        double b_l_w = r_dist*t_dist;
        double b_r_w = l_dist*t_dist;
        double sum_weights;

        if (ValidCoord(l_x, t_y)) {
            tl = GetPixel(l_x, t_y);
            num_sampled++;
        }
        else {
            t_l_w = 0.0;
        }
        if (ValidCoord(l_x, b_y)) {
            bl = GetPixel(l_x, b_y);
            num_sampled++;
        }
        else {
            b_l_w = 0.0;
        }
        if (ValidCoord(r_x, t_y)) {
            tr = GetPixel(r_x, t_y);
            num_sampled++;
        }
        else {
            t_r_w = 0.0;
        }
        if (ValidCoord(r_x, b_y)) {
            br = GetPixel(r_x, b_y);
            num_sampled++;
        }
        else {
            b_r_w = 0.0;
        }
        sum_weights = t_l_w + b_l_w + t_r_w + b_r_w;
        if (sum_weights != 1) {
            double fix_factor = 1 / sum_weights;
            t_l_w = t_l_w * fix_factor;
            b_l_w = b_l_w * fix_factor;
            t_r_w = t_r_w * fix_factor;
            b_r_w = b_r_w * fix_factor;
        }

        tl = tl * t_l_w;
        tr = tr * t_r_w;
        bl = bl * b_l_w;
        br = br * b_r_w;
        Pixel result = tl + tr + bl + br;
        return result;
    }
    if (sampling_method == IMAGE_SAMPLING_GAUSSIAN) {
        int source_x = u;
        int source_y = v;
        float** kernel = CreateGaussian(1, 1);
        int e_x, e_y;
        double new_r = 0.0;
        double new_g = 0.0;
        double new_b = 0.0;
        if (ValidCoord(source_x, source_y)) {
            for (e_y = 0; e_y < 3; e_y++) {
                for (e_x = 0; e_x < 3; e_x++) {
                    source_x = source_x + e_x - 1;
                    source_y = source_y + e_y - 1;
                    if ((source_x > 0) && (source_x < Width()) && (source_y > 0) && (source_y < Height()));
                    else {
                        if (source_x < 0 || source_x >= Width()) {
                            source_x = source_x - (e_x - 1);
                        }
                        if (source_y < 0 || source_y >= Height()) {
                            source_y = source_y - (e_y - 1);
                        }
                    }
                    Pixel p_1 = GetPixel(source_x, source_y);
                    float red = p_1.r;
                    float green = p_1.g;
                    float blue = p_1.b;
                    new_r = new_r + (red * kernel[e_x][e_y]);
                    new_g = new_g + (green * kernel[e_x][e_y]);
                    new_b = new_b + (blue * kernel[e_x][e_y]);
                }
            }
            Pixel result = Pixel((int)new_r, (int)new_g, (int)new_b);
            return result;
        }
        else {
            return Pixel(0, 0, 0);
        }
    }

    return Pixel();
}