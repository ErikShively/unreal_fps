// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPawn.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"
#include "Misc/App.h"

#define PLAYER_HEIGHT 64
#define PLAYER_WIDTH 32
#define MOUSE_SENS 1
#define PLAYER_ACCEL 2650.f
#define PLAYER_SPEED 600.f
#define PLAYER_FRICTION 10.f //Comes to a complete stop in 1/n seconds.
#define GROUNDED_LENGTH 10
#define SUBSTEPS 4
#define WALL_PUSH 1

// Constructor
AMyPawn::AMyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocation(FVector(0, 0, 150));
	Hitbox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hitbox"));
	Hitbox->SetupAttachment(RootComponent);
	Hitbox->SetRelativeLocation(FVector(0, 0, PLAYER_HEIGHT));

	State = -1;

	//FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(PLAYER_WIDTH, PLAYER_HEIGHT/2);
	//CollisionShape.MakeCapsule(PLAYER_WIDTH,PLAYER_HEIGHT/2);

}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();

	
	
}

// Called every frame
void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//Handle state logic here.
	//Maybe use boxes to handle collision

	//if (GEngine) {
	//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("State: %i"), State));
	//}
	
	switch (State)
	{
	case -1: //Default
		DefaultState(DeltaTime);
		break;
	case 1: //Grounded Movement
		MoveGrounded(DeltaTime);;
		break;
	case 2: //Air Movement
		MoveAir(DeltaTime);
		break;
	default: //Just a safety catch-all to default.
		DefaultState(DeltaTime);
		break;
	}
	
}

FHitResult AMyPawn::CheckGrounded(float DeltaTime)//Try a pass by reference here to store the results in a container object. This lets you reliably return a bool while also giving you the results.
{
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0, 0, -GROUNDED_LENGTH * 2);
	FHitResult OutHit;
	FVector BoxExtent = FVector(PLAYER_WIDTH * 1.5, PLAYER_WIDTH * 1.5, GROUNDED_LENGTH);
	FCollisionQueryParams CollisionParams;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent * 2);

	//DrawDebugBox(GetWorld(), GetActorLocation() + FVector(0, 0, -GROUNDED_LENGTH * 2), BoxExtent, FColor::Red, false);
	if (GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_Visibility, CollisionShape))
	{
		return OutHit;
	}
	else
	{
		return OutHit;
	}
}

bool AMyPawn::CheckGroundedBool(float DeltaTime, FHitResult &InHit)
{
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0, 0, -GROUNDED_LENGTH * 2);
	FVector BoxExtent = FVector(PLAYER_WIDTH * 1.5, PLAYER_WIDTH * 1.5, GROUNDED_LENGTH);
	FCollisionQueryParams CollisionParams;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent * 2);

	//DrawDebugBox(GetWorld(), GetActorLocation() + FVector(0, 0, -GROUNDED_LENGTH * 2), BoxExtent, FColor::Red, false);
	if (GetWorld()->SweepSingleByChannel(InHit, Start, End, FQuat::Identity, ECC_Visibility, CollisionShape))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void AMyPawn::DefaultState(float DeltaTime)
{
	FHitResult GHit = CheckGrounded(DeltaTime);
	if(CheckGroundedBool(DeltaTime, GHit))
	{
		State = 1;
	}
	else
	{
		State = 2;
	}
	

}

void AMyPawn::MoveGrounded(float DeltaTime) //Maybe instead of generating the velocity then colliding, do the collision first and modify the velocity.
{
	//Before handling movement logic, we need to snap to the floor.
	FHitResult GHit; //Don't snap if there's no detected floor.
	bool HitGround = CheckGroundedBool(DeltaTime, GHit);
	FVector EndVec;
	int Z = GHit.ImpactPoint.Z + GROUNDED_LENGTH * 2;
	
	Velocity.Y += Input_Vec.Y ? Input_Vec.Y * PLAYER_ACCEL * DeltaTime : -Velocity.Y * PLAYER_FRICTION * DeltaTime;
	Velocity.X += Input_Vec.X ? Input_Vec.X * PLAYER_ACCEL * DeltaTime : -Velocity.X * PLAYER_FRICTION * DeltaTime;
	Velocity = Velocity.FVector::GetClampedToSize(0,PLAYER_SPEED);
	Cur_Ang.Yaw += (Cur_Ang_Vel.Yaw); //Is mouse automatically adjusting for frame time?
	Cur_Ang.Pitch += (Cur_Ang_Vel.Pitch);
	For_Vel.Y = (sinf(2 * PI * Cur_Ang.Yaw / 360));
	For_Vel.X = (cosf(2 * PI * Cur_Ang.Yaw / 360));

	Strafe_Vel.Y = (sinf(2 * PI * (Cur_Ang.Yaw + 90) / 360));
	Strafe_Vel.X = (cosf(2 * PI * (Cur_Ang.Yaw + 90) / 360));

	Cur_Vel = (Strafe_Vel * Velocity.X + For_Vel * Velocity.Y).FVector::GetClampedToMaxSize(PLAYER_SPEED);
	//EndVec = Collide(DeltaTime, Cur_Vel * DeltaTime) + GetActorLocation();
	EndVec = Collide2(DeltaTime, Cur_Vel * DeltaTime);
	EndVec = FVector(EndVec.X, EndVec.Y, HitGround ? Z : EndVec.Z);
	SetActorLocation(EndVec);

	Camera->SetWorldRotation(Cur_Ang);
	
	if (HitGround == false)
	{
		State = 2;
	}
}

void AMyPawn::MoveAir(float DeltaTime)
{
	FVector EndVec;
	FHitResult GHit;
	EndVec = Collide(DeltaTime,GetActorLocation() + Cur_Vel * DeltaTime); //This is going to have to be looked at. Don't use addlocaloffset.
	SetActorLocation(EndVec);
	AddActorLocalOffset(FVector(0,0,-200*DeltaTime));
	Cur_Ang.Yaw += (Cur_Ang_Vel.Yaw);
	Cur_Ang.Pitch += (Cur_Ang_Vel.Pitch);
	Camera->SetWorldRotation(Cur_Ang);
	if (CheckGroundedBool(DeltaTime, GHit))
	{
		State = 1;
	}
	else
	{
		State = 2;
	}
}

FVector AMyPawn::Collide(float DeltaTime, FVector Target) //tbqh idk maybe just design around this collision. it works well enough
{
	//Maybe don't scan at the predicted point, but rather at the current position. Also try making the scan a box again.
	//This should help the corner problem.
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(PLAYER_HEIGHT, PLAYER_WIDTH); //Try a box?
	FCollisionShape CollisionShapeBase = FCollisionShape::MakeCapsule(PLAYER_HEIGHT, PLAYER_WIDTH*0.9); //Try a box?
	//FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent);
	TArray<FHitResult> Hit;
	FVector CapsulePointBase = RootComponent->GetComponentLocation() + FVector(0, 0, PLAYER_HEIGHT);
	FVector CapsulePoint = CapsulePointBase + Target;
	FVector FTarget = Target;

	DrawDebugCapsule(GetWorld(), CapsulePointBase, PLAYER_HEIGHT, PLAYER_WIDTH, FQuat::Identity, FColor::Red, false);
	DrawDebugCapsule(GetWorld(), CapsulePoint, PLAYER_HEIGHT, PLAYER_WIDTH, FQuat::Identity, FColor::Green, false);

	if (GetWorld()->SweepMultiByChannel(Hit, CapsulePointBase, CapsulePointBase, FQuat::Identity, ECC_WorldStatic, CollisionShape))
	{
		//We need to get players unstuck if the enter the geometry for whatever reason. This teleports them out.
		FVector DisplacementSum = FVector(0,0,0);
		for (auto& iHit : Hit)
		{
			DisplacementSum += iHit.ImpactNormal;
		}
		FTarget = DisplacementSum * WALL_PUSH; //Make this the new target and then handle the logic accordingly.
	}

	if (GetWorld()->SweepMultiByChannel(Hit, CapsulePoint, CapsulePoint, FQuat::Identity, ECC_WorldStatic, CollisionShape))
	{
		FVector WorkingPosition = FTarget;
		for (auto& iHit : Hit)
		{
			FPlane SlidePlane = FPlane(iHit.ImpactPoint, iHit.ImpactNormal);
			WorkingPosition = FVector::VectorPlaneProject(WorkingPosition, SlidePlane);
		}
		return WorkingPosition;
	}
	else
	{
		return Target;
	}
}

FVector AMyPawn::Collide2(float DeltaTime, FVector Target)
{
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(PLAYER_HEIGHT, PLAYER_WIDTH); //Try a box?
	TArray<FHitResult> Hit;
	FVector CapsulePointBase = GetActorLocation() + FVector(0, 0, PLAYER_HEIGHT);
	FVector CapsulePoint = CapsulePointBase + Target;
	FVector FTarget = Target;

	DrawDebugCapsule(GetWorld(), CapsulePointBase, PLAYER_HEIGHT, PLAYER_WIDTH, FQuat::Identity, FColor::Red, false);
	DrawDebugCapsule(GetWorld(), CapsulePoint, PLAYER_HEIGHT, PLAYER_WIDTH, FQuat::Identity, FColor::Green, false);

	if (GetWorld()->SweepMultiByChannel(Hit, CapsulePoint, CapsulePoint, FQuat::Identity, ECC_WorldStatic, CollisionShape))
	{
		FHitResult iHit = Hit[0];
		//FVector HitPoint = iHit.ImpactPoint - GetActorLocation();
		//FPlane HPlane = FPlane(iHit.ImpactPoint, iHit.ImpactNormal);
		//FVector EndVec = FVector::VectorPlaneProject(FTarget, HPlane);
		//Cur_Vel = FVector(0,0,0);
		//FVector EndVec = HitPoint.FVector::ProjectOnTo(FTarget);
		return iHit.Location;
	}
	else
	{
		return Target + GetActorLocation();
	}
}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("MoveX", this, &AMyPawn::MoveX);
	InputComponent->BindAxis("MoveY", this, &AMyPawn::MoveY);
	InputComponent->BindAxis("MouseX", this, &AMyPawn::MouseX);
	InputComponent->BindAxis("MouseY", this, &AMyPawn::MouseY);
}

void AMyPawn::MoveX(float val)
{
	Input_Vec.X = FMath::IsNearlyZero(val) ? 0 : val;
}

void AMyPawn::MoveY(float val)
{
	Input_Vec.Y = FMath::IsNearlyZero(val) ? 0 : val;
}

void AMyPawn::MouseX(float val)
{
	Cur_Ang_Vel.Yaw = val * MOUSE_SENS;
}

void AMyPawn::MouseY(float val)
{
	Cur_Ang_Vel.Pitch = val * MOUSE_SENS;
}