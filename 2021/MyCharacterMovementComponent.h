// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine.h"
#include "MyCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_BASE = 0,
	CMOVE_JET = 1,
	CMOVE_DASH = 2,
	CMOVE_CROUCH = 3
};
//MoveSafeUpdatedComponent((Velocity * deltaTime), rotation quaternion, bool, fhitresult)
//This is going to be your main movement function?
UCLASS()
class FPS_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	

public: //Vars
	bool bWantsToBase;
	bool bWantsToJet;
	bool bWantsToDash;
	bool bWantsToCrouch;

	float Energy;

private:
	FVector JetVector;
	FVector DashVector;
	bool validMove;
	float dashTime;

public: //Overrides
	UMyCharacterMovementComponent();
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity) override; //To check state transitions
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override; //To handle state transitions
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

public: //Comepletely new functions.
	//Maybe add a function to handle state switching completely gracefully. Not sure if one of the overrides handle it well.

	void PhysBase(float deltaTime, int32 Iterations);
	void PhysJet(float deltaTime, int32 Iterations);
	void PhysDash(float deltaTime, int32 Iterations);
	void PhysCrouch(float deltaTime, int32 Iterations);
	
	//These should be changed to behave more like setters/getters instead of doing a check every time.
	//Maybe perform a function to check elegibility eg checkCanBase()
	bool canBase();
	bool canJet();
	bool canDash();
	bool canCrouch();

	void setCanBase();
	void setCanJet();
	void setCanDash();
	void setCanCrouch();

	void DashCooldown();

	void changeMode();
	void printDebugMessage();
	void printDynamicDebugMessage();

	bool IsCustomMovementMode(uint8 cm) const;
};