// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/WORDSOULAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "EngineUtils.h"


AWORDSOULPlayerController::AWORDSOULPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
}

AWORDSOULPlayerController::~AWORDSOULPlayerController()
{

}

void AWORDSOULPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsConnected) return;

	AWORDSOULCharacter* MyCharacter = Cast<AWORDSOULCharacter>(this->GetPawn());
	if (!MyCharacter) return;

	FVector MyLoc = MyCharacter->GetActorLocation();
	float GroundSpeed = MyCharacter->GetGroundSpeed();
	char flag = NULL;
	float MyRot = MyCharacter->GetActorRotation().Yaw;
	if (MyCharacter->GetIsFalling())
	{
		flag = 0b0000'0001;
	}
	if ((MyCharacter->GetActionState()) == EActionState::EAS_Dodge)
	{
		flag |= 0b000'0100;
	}
	Socket->SendCharacterInfo(MyLoc, GroundSpeed, flag, MyRot);
	
	UpdatePlayerInfo(OtherCharacterInfo);
}

void AWORDSOULPlayerController::RecvCharacterInfo(const SCPosition& CharacterInfo)
{
	OtherCharacterInfo = CharacterInfo;
}

void AWORDSOULPlayerController::BeginPlay()
{
	Super::BeginPlay();

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

	/*CurrentLocation = this->GetPawn()->GetActorLocation();
	PreviousLocation = FVector(0.f, 0.f, 0.f);*/

	Socket->Party();
	Socket->StartRecvThread();
}

void AWORDSOULPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	Socket->EndRecvThread();
}

void AWORDSOULPlayerController::UpdatePlayerInfo(const SCPosition& CharacterInfo)
{

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* cPawn = *It;
		AWORDSOULCharacter* cCharacter = Cast<AWORDSOULCharacter>(cPawn);
		if (cCharacter and cCharacter == OtherCharacter)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Player x y z : %f %f %f"), CharacterInfo.x, CharacterInfo.y, CharacterInfo.z);
			FVector NewLocation = FVector(CharacterInfo.x, CharacterInfo.y, CharacterInfo.z);

			cCharacter->SetActorLocation(NewLocation);
			cCharacter->SetActorRotation(FRotator(0.f, CharacterInfo.r, 0.f));

			/*CurrentLocation = NewLocation;
			if (CurrentLocation != PreviousLocation)
			{
				FVector DirectionVector = FVector(CurrentLocation.X - PreviousLocation.X, CurrentLocation.Y - PreviousLocation.Y, 0.f);

				CharacterRotation = UKismetMathLibrary::FindLookAtRotation(FVector::ZeroVector, DirectionVector);
				cCharacter->SetActorRotation(CharacterRotation);

				PreviousLocation = CurrentLocation;
			}*/

			
			UAnimInstance* AnimInst = cCharacter->GetMesh()->GetAnimInstance();
			UWORDSOULAnimInstance* WORDSOULAnimInst = Cast<UWORDSOULAnimInstance>(AnimInst);
			if (WORDSOULAnimInst)
			{
				WORDSOULAnimInst->GroundSpeed = CharacterInfo.v;
				if ((CharacterInfo.flag & 0b0000'0011) == 0b01) //jumping(IsFalling)
				{
					WORDSOULAnimInst->IsFalling = true;
					UE_LOG(LogTemp, Warning, TEXT("JUMP : TRUE"));
				}
				else
				{
					WORDSOULAnimInst->IsFalling = false;
				}

				if ((CharacterInfo.flag & 0b0000'0100) != 0)  // dodge
				{
					cCharacter->PlayDodgeMontage();
				}
			}
			break;
		}
	}
}
