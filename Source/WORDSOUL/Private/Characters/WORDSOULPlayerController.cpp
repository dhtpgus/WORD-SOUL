// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"


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

	if (OtherCharacterInfo)
	{
		UpdatePlayerInfo(*OtherCharacterInfo);
	}
	

}

void AWORDSOULPlayerController::RecvCharacterInfo(SCCharacterInfo* CharacterInfo)
{
	OtherCharacterInfo = CharacterInfo;
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

void AWORDSOULPlayerController::UpdatePlayerInfo(const SCCharacterInfo& CharacterInfo)
{

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* cPawn = *It;
		AWORDSOULCharacter* cCharacter = Cast<AWORDSOULCharacter>(cPawn);
		if (cCharacter and cCharacter->PlayerID == OtherCharacter->PlayerID) // characterinfo에서 누구의 좌표인지 구분해주는 id 필요
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerID : %d"), cCharacter->PlayerID);
			FVector NewLocation = FVector(CharacterInfo.x, CharacterInfo.y, CharacterInfo.z); // 자신의 위치를 동기화하게됨

			cCharacter->SetActorLocation(NewLocation);
			break;
		}
	}
}
