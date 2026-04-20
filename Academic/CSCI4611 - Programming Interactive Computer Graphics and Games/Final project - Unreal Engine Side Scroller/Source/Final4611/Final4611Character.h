// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Final4611Character.generated.h"

UCLASS(config=Game)
class AFinal4611Character : public ACharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Function, meta = (AllowPrivateAccess = "true"))
        bool doubleJump;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Function, meta = (AllowPrivateAccess = "true"))
        bool tripleJump;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Function, meta = (AllowPrivateAccess = "true"))
        bool tJumpActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Function, meta = (AllowPrivateAccess = "true"))
        bool wallJump;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Function, meta = (AllowPrivateAccess = "true"))
        bool sliding;

    UFUNCTION()
        void MyStartJump();

    UFUNCTION()
        void MyStopJump();

    UFUNCTION(BlueprintCallable, Category = "TripleJump")
        void MyStartTripleJump();

    UFUNCTION(BlueprintCallable, Category = "TripleJump")
        void MyStopTripleJump();

    UFUNCTION()
        void MyStartSprint();

    UFUNCTION()
        void MyStopSprint();

    UFUNCTION()
        void MyStartSlide();

    UFUNCTION()
        void MyStopSlide();

    UFUNCTION()
        void MyStartWallJump();

    UFUNCTION()
        void MyStopWallJump();

    float time_since_slide = 1.0f;

protected:


	/** Called for side to side input */
	void MoveRight(float Val);

	/** Handle touch inputs. */
	void TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location);

	/** Handle touch stop event. */
	void TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location);

    /**call required methods on land */
    virtual void Landed(const FHitResult & Hit) override;

    /**call proper inputs per frame*/
    virtual void Tick(float DeltaTime) override;

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface


public:
	AFinal4611Character();
	
        /** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};
