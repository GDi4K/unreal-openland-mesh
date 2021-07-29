// Fill out your copyright notice in the Description page of Project Settings.


#include "OpenLandMeshHelloWorldActor.h"


// Sets default values
AOpenLandMeshHelloWorldActor::AOpenLandMeshHelloWorldActor()
{
	SubDivisions = 5;
	SmoothNormalAngle = 90;
	bRunCpuVertexModifiers = true;
	bAnimate = true;
}

// Called when the game starts or when spawned
void AOpenLandMeshHelloWorldActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOpenLandMeshHelloWorldActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVertexModifierResult AOpenLandMeshHelloWorldActor::OnModifyVertex_Implementation(FVertexModifierPayload Payload)
{
	const FVector Normal = Payload.Position.GetSafeNormal();
    FVector Position = Normal * 100;

    const float Distance = FVector::Distance(Position, FVector(0, 0, 90));
    const float HeightRange = FMath::Sin(Distance * 0.2 + Payload.TimeInSeconds * 8) * 0.5 + 0.5;
    Position += Normal * HeightRange * 20 * (Distance * 0.01);
    		
    return {
    	Position
    };
}

UOpenLandMeshPolygonMeshProxy* AOpenLandMeshHelloWorldActor::GetPolygonMesh_Implementation()
{
	UOpenLandMeshPolygonMeshProxy* P = NewObject<UOpenLandMeshPolygonMeshProxy>();
	
	const FVector A = FVector(-50, -50, 0);
	const FVector B = FVector(-50, 50, 0);
	const FVector C = FVector(50, 50, 0);
	const FVector D = FVector(50, -50, 0);
	const FVector Top = FVector(0, 0, 50);
	const FVector Bottom = FVector(0, 0, -50);
		
	P->AddTriFace(A, Top, D);
	P->AddTriFace(B, Top, A);
	P->AddTriFace(C, Top, B);
	P->AddTriFace(D, Top, C);
	P->AddQuadFace(A, D, C, B);

	return P;
}


