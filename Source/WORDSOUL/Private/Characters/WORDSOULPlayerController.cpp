// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULPlayerController.h"
#include "Kismet/GameplayStatics.h"

AWORDSOULPlayerController::AWORDSOULPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

	Socket = ClientSocket::GetInstance();
	Socket->InitSocket();
	bIsConnected = Socket->ConnectToServer(SERVER_IP, SERVER_PORT);
	if (bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER CONNECT SUCCESS"));
		Socket->SetPlayerController(this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER CONNECT FAILED"));
	}
}

AWORDSOULPlayerController::~AWORDSOULPlayerController()
{

}

void AWORDSOULPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsConnected) return;

	Socket->SendCharacterLocation(this->GetPawn()->GetActorLocation());
}

void AWORDSOULPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//AWORDSOULCharacter Player = Cast<AWORDSOULCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	Socket->Party();
	Socket->StartRecvThread();
}

void AWORDSOULPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Socket->EndRecvThread();
}
