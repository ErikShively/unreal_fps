// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"


UCLASS()
class PROJECT0_API AMyPawn : public APawn
{
	GENERATED_BODY()

	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Hitbox;

	
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;


protected:
	FVector Input_Vec;
	FVector For_Vel;
	FVector Strafe_Vel;

	FVector Velocity;
	FVector DesVec;
	FVector Cur_Vel;

	FVector SlideVector;

	FRotator Cur_Ang;
	FRotator Cur_Ang_Vel;

	FCollisionShape CollisionShape;

	TArray<FHitResult> OutHit;

	float VelScale;

	int State;

public:
	// Sets default values for this pawn's properties
	AMyPawn();
	void MoveX(float val);
	void MoveY(float val);
	void MouseX(float val);
	void MouseY(float val);

	void DefaultState(float DeltaTime);
	bool CheckGroundedBool(float DeltaTime, FHitResult &InHit);
	FHitResult CheckGrounded(float DeltaTime);

	void MoveGrounded(float DeltaTime);
	void MoveAir(float DeltaTime);

	FVector Collide(float DeltaTime, FVector Target);
	FVector Collide2(float DeltaTime, FVector Target);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE class UStaticMeshComponent* GetHitbox() const { return Hitbox; }
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
};
