// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacterMovementComponent.h"
//Constructor
UMyCharacterMovementComponent::UMyCharacterMovementComponent()
{
	bWantsToBase = false;
	bWantsToJet = false;
	bWantsToDash = false;
	bWantsToCrouch = false;

	Energy = 600.f;
}

//Overrides
void UMyCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (GetPawnOwner()->IsLocallyControlled())
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString("Energy: ") + FString::SanitizeFloat(Energy), true, false, FLinearColor::Blue, 0.0);
		UKismetSystemLibrary::PrintString(GetWorld(), FString("Velocity: ") + (Velocity.ToString()), true, false, FLinearColor::Green, 0.0);
		UKismetSystemLibrary::PrintString(GetWorld(), FString("Speed: ") + FString::SanitizeFloat(Velocity.Size()), true, false, FLinearColor::Red, 0.0);
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMyCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector & OldLocation, const FVector & OldVelocity)
{
	//Handle the transitions from the flags here
	//I think cancels will also play a factor here.
	if (bWantsToBase)
	{
		if (canBase())
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_BASE);
		}
	}

	if (bWantsToJet)
	{
		if (canJet())
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_JET);
		}
	}

	if (bWantsToDash)
	{
		if (canDash()) 
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_DASH);
		}
	}

	if (bWantsToCrouch)
	{
		if (canCrouch())
		{
			SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_CROUCH);
		}
	}
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
}

void UMyCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	//You may have to set a variable to keep track of which movement state is active here.
	//It could also work in the implementations of the states themselves: PhysWalk or whatever else.
	//Fixed lol
	if (PreviousMovementMode == MovementMode && PreviousCustomMode == CustomMovementMode)
	{
		Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
		return;
	}
	//Leaving states
	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == ECustomMovementMode::CMOVE_JET)
	{
		JetVector = FVector(0, 0, 0);
		MaxWalkSpeed = 600;
	}

	if (PreviousMovementMode == EMovementMode::MOVE_Custom && PreviousCustomMode == ECustomMovementMode::CMOVE_DASH)
	{
		JetVector = FVector(0, 0, 0);
		MaxWalkSpeed = 600;
	}

	//Entering States

	if (MovementMode == EMovementMode::MOVE_Walking)
	{
		Energy = 600.f; //Later implement a function to restore this gradually but this will do for now.
	}

	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_JET))
	{
		validMove = false;
		if (Energy > 50)
		{
			Energy -= 50;
			validMove = true;
		}	
		dashTime = 1000.f;
		FRotator Heading = GetPawnOwner()->GetControlRotation();
		JetVector = (FRotator(0, Heading.Yaw, 0).Vector()); //Magnitude probably requires a blueprint UPROPERTY. Figure out how to do that.
		Velocity = JetVector * 800;
	}

	if (IsCustomMovementMode(ECustomMovementMode::CMOVE_DASH))
	{
		FRotator Heading = GetPawnOwner()->GetControlRotation();
		DashVector = ((FRotator(0, Heading.Yaw, 0).Vector() * 500) + FVector(Velocity.X, Velocity.Y, 0).Size()); //Consider stripping velocity of its z axis and then taking the magnitude of that and using it as the dash vector size.
		if (Energy > 0)
			Velocity = Velocity;
	}
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UMyCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == ECustomMovementMode::CMOVE_BASE)
	{
		PhysBase(deltaTime, Iterations);
	}
	if (CustomMovementMode == ECustomMovementMode::CMOVE_JET)
	{
		PhysJet(deltaTime, Iterations);
	}
	if (CustomMovementMode == ECustomMovementMode::CMOVE_DASH)
	{
		PhysDash(deltaTime, Iterations);
	}
	if (CustomMovementMode == ECustomMovementMode::CMOVE_CROUCH)
	{
		PhysCrouch(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

//New

//Maybe have a table for state compatability
bool UMyCharacterMovementComponent::canBase()
{
	return true;
}

bool UMyCharacterMovementComponent::canJet()
{
	if (MovementMode == EMovementMode::MOVE_Walking)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool UMyCharacterMovementComponent::canCrouch()
{
	if (MovementMode == EMovementMode::MOVE_Walking)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UMyCharacterMovementComponent::canDash()
{
	if (MovementMode == EMovementMode::MOVE_Falling)
		return true;
	else
		return false;
}

//Phys
void UMyCharacterMovementComponent::PhysBase(float deltaTime, int32 Iterations)
{
	
}

void UMyCharacterMovementComponent::PhysJet(float deltaTime, int32 Iterations)
{
	FHitResult outHit;
	//Should probably generate bezier function for dash impulse.

	//Single-time activation. Not cancelled by release.
	//Direction is static from activation.
	//SafeMoveUpdatedComponent(Velocity * deltaTime, FRotator(0, GetPawnOwner()->GetControlRotation().Yaw, 0), true, outHit);
	//Consume a static amount of energy
	//Also figure out how to restrict camera movement.
	//Also add a cooldown

	//Find a way to initialize variables for one function call only
	//Just use the other function that's meant for state transitions
	if (validMove)
	{
		dashTime = FMath::Clamp(dashTime, 0.f, 100.f);
		dashTime -= 100 * deltaTime; //100 per second
		SafeMoveUpdatedComponent(Velocity * deltaTime, FRotator(0, GetPawnOwner()->GetControlRotation().Yaw, 0), true, outHit);
		//Consider adding a bonk here
		if (dashTime <= 0)
		{
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(deltaTime, Iterations);
		}
	}
	
	
}

void UMyCharacterMovementComponent::PhysDash(float deltaTime, int32 Iterations)
{
	//This needs to be moved to a grounded dash. 
	//Air dash, if it's implemented, should be in a seperate state.
	FHitResult outHit;
	FHitResult metaCapsule;
	//Cast metaCapsule before working with it. Cannot slide without casting a new capsule.
	//If there's a hit with outHit, swap to falling. Also consider slidealongsurface
	//CapsuleTouched
	if (bWantsToDash)
	{
		Energy = FMath::Clamp(Energy, 0.f, 600.f);
		if (Energy > 0)
		{
				Energy -= 300 * deltaTime;
				Velocity += DashVector * deltaTime;
				printDebugMessage();
				SafeMoveUpdatedComponent(Velocity * deltaTime, FRotator(0, GetPawnOwner()->GetControlRotation().Yaw, 0), true, outHit);
				if (GetPawnOwner()->IsLocallyControlled())
				{
					UKismetSystemLibrary::PrintString(GetWorld(), FString("OutHit Distance: ") + FString::SanitizeFloat(outHit.Distance), true, false, FLinearColor::Blue, 0.0);
				}
				//maybe make a slightly larger area around the capsule to catch things like this.
				
		}
		else //Once it hits 0
		{
			SetMovementMode(EMovementMode::MOVE_Falling);
			StartNewPhysics(deltaTime, Iterations);
		}
	}
	else
	{
		SetMovementMode(EMovementMode::MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
	}
}

void UMyCharacterMovementComponent::PhysCrouch(float deltaTime, int32 Iterations)
{
	//May have to look up how to change hitbox size.
	//Like any other grounded state, it needs to be swapped to a falling state where it would normally apply. Not sure if I could create a normal grounded state to inherit from
	//Or if there's a straightforward way to check within the state.
	//Consider overriding the basic walking state. Also MoveAlongFloor() FindFloor()
	if (bWantsToCrouch)
	{
		//Implement the slide here. Inheret velocity but do not generate any. Turns can kill velocity, so you'd have to be careful not to turn too sharply.
		//dv/dt = -X(dtheta/dt) or something to that effect.
		//If there's a hit with outHit, swap to falling.
	}
	else
	{
		SetMovementMode(EMovementMode::MOVE_Falling); 
		StartNewPhysics(deltaTime, Iterations);
	}

}

//You may also have to override the falling state. Also look into BoostAirControl() method. LimitAirControl() AirControl.

//Debug
void UMyCharacterMovementComponent::changeMode()
{
	SetMovementMode(EMovementMode::MOVE_Custom, ECustomMovementMode::CMOVE_BASE);
}

void UMyCharacterMovementComponent::printDebugMessage()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString::Printf(TEXT("Hey check out this debug message. It's new!")));
	}
}


void UMyCharacterMovementComponent::printDynamicDebugMessage()
{
	if (GetPawnOwner()->IsLocallyControlled())
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString("This is a dynamic debug message."), true, false, FLinearColor::Blue, 0.0);
	}
}

bool UMyCharacterMovementComponent::IsCustomMovementMode(uint8 cm) const
{
	if (MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == cm)
		return true;
	return false;
}
