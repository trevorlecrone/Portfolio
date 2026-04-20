// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Final4611Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AFinal4611Character::AFinal4611Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->SocketOffset = FVector(0.f,0.f,75.f);
	CameraBoom->RelativeRotation = FRotator(0.f,180.f,0.f);

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFinal4611Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFinal4611Character::MyStartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AFinal4611Character::MyStopJump);
    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AFinal4611Character::MyStartSprint);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AFinal4611Character::MyStopSprint);
    PlayerInputComponent->BindAction("Slide", IE_Pressed, this, &AFinal4611Character::MyStartSlide);
    PlayerInputComponent->BindAction("Slide", IE_Released, this, &AFinal4611Character::MyStopSlide);
    PlayerInputComponent->BindAction("WallJump", IE_Pressed, this, &AFinal4611Character::MyStartWallJump);
    PlayerInputComponent->BindAction("WallJump", IE_Released, this, &AFinal4611Character::MyStopWallJump);


    PlayerInputComponent->BindAxis("MoveRight", this, &AFinal4611Character::MoveRight);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFinal4611Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFinal4611Character::TouchStopped);
    
    doubleJump = true;
}

// Called every frame
void AFinal4611Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!wallJump && GetCharacterMovement()->AirControl < 0.80f) {
        GetCharacterMovement()->AirControl += (DeltaTime * 0.6f);
        if(GetCharacterMovement()->AirControl > 0.80f){
            GetCharacterMovement()->AirControl = 0.80f;
        }
    }
    if (time_since_slide < 0.6f && sliding) {
        time_since_slide += DeltaTime;
        if (time_since_slide >= 0.5f) {
            sliding = false;
        }
    }

}

void AFinal4611Character::MoveRight(float Value)
{
	// add movement in that direction
	AddMovementInput(FVector(0.f,-1.f,0.f), Value);
}

void AFinal4611Character::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
    MyStartJump();
}

void AFinal4611Character::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
    MyStopJump();
}

void AFinal4611Character::MyStartJump() 
{
    if (GetCharacterMovement()->IsMovingOnGround()) {
        bPressedJump = true;
        LaunchCharacter(FVector(0, 0, 1) * GetCharacterMovement()->JumpZVelocity, false, false);
        return;
    }
    else if(doubleJump) {
        bPressedJump = true;
        float cZVel = GetVelocity().Z > 0 ? -GetVelocity().Z * 0.65 : -GetVelocity().Z;
        LaunchCharacter(FVector(0, 0, 1) * (cZVel + GetCharacterMovement()->JumpZVelocity*0.8), false, false);
        doubleJump = false;
        return;
    }
    else if (tripleJump && !doubleJump) {
        bPressedJump = true;
        float cZVel = GetVelocity().Z > 0 ? -GetVelocity().Z * 0.65 : -GetVelocity().Z;
        LaunchCharacter(FVector(0, 0, 1) * (cZVel + GetCharacterMovement()->JumpZVelocity*0.8), false, false);
        tripleJump = false;
        return;
    }
}

void AFinal4611Character::MyStopJump()
{
    bPressedJump = false;
    return;
}

void AFinal4611Character::MyStartTripleJump()
{
    tJumpActive = true;
    tripleJump = true;
}

void AFinal4611Character::MyStopTripleJump()
{
    tJumpActive = false;
    tripleJump = false;
}

void AFinal4611Character::MyStartSprint()
{
    if (GetCharacterMovement()->IsMovingOnGround()) {
        GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
        return;
    }

}
void AFinal4611Character::MyStopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    return;
}

void AFinal4611Character::MyStartSlide()
{
    if (GetCharacterMovement()->IsMovingOnGround()) {
        sliding = true;
        time_since_slide = 0.0f;
        return;
    }

}
void AFinal4611Character::MyStopSlide()
{
    if (sliding) {
        sliding = false;
        return;
    }
}

void AFinal4611Character::MyStartWallJump()
{
    
    FHitResult Hit;

    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

    TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
    if (!GetCharacterMovement()->IsMovingOnGround()) {

        FVector zeroCVel = GetVelocity() * -1.0f;
        FVector newVel = zeroCVel + (Hit.ImpactNormal * 400) + FVector(0, 0, GetCharacterMovement()->JumpZVelocity);

        if (GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), GetActorLocation() - FVector(0, 100, 0), TraceObjectTypes)) {
            wallJump = true;
            FRotator CRotation = GetActorRotation();
            CRotation.Yaw = 90;
            SetActorRotation(CRotation);
            FVector newVel = zeroCVel + (Hit.ImpactNormal * 400) + FVector(0, 0, GetCharacterMovement()->JumpZVelocity);
            GetCharacterMovement()->AirControl = 0.40f;
            LaunchCharacter(newVel, false, false);
        }
        else if (GetWorld()->LineTraceSingleByObjectType(Hit, GetActorLocation(), GetActorLocation() + FVector(0, 100, 0), TraceObjectTypes)) {
            wallJump = true;
            FRotator CRotation = GetActorRotation();
            CRotation.Yaw = -90;
            SetActorRotation(CRotation);
            FVector newVel = zeroCVel + (Hit.ImpactNormal * 400) + FVector(0, 0, GetCharacterMovement()->JumpZVelocity);
            GetCharacterMovement()->AirControl = 0.40f;
            LaunchCharacter(newVel, false, false);
        }
    }
}

void AFinal4611Character::MyStopWallJump()
{
    wallJump = false;
}

void AFinal4611Character::Landed(const FHitResult & Hit)
{
    doubleJump = true;
    if (tJumpActive) {
        tripleJump = true;
    }
    Super::Landed(Hit);
    return;
}
