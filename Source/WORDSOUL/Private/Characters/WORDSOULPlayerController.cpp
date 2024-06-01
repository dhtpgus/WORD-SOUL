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

	SendPlayerInfo();
	
	SpawnMonster(NewEntityInfo);

	UpdatePlayerInfo(OtherCharacterInfo);
	UpdateMonsterInfo(EnemyInfo);
}

void AWORDSOULPlayerController::SpawnMonster(const SCNewEntity& EntityInfo)
{
	if ((EntityInfo.id != 0xFFFF and EntityInfo.id != 0xFFFE) and (EntityInfo.flag & 0b0000'0011) == 0 and EntityInfo.entity_type == 2) // spawn monster
	{
		FVector MonsterSpawnLocation = FVector(EntityInfo.x, EntityInfo.y, EntityInfo.z);
		FRotator MonsterSpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters MonsterSpawnParams;
		MonsterSpawnParams.Owner = this;
		MonsterSpawnParams.Instigator = GetInstigator();
		AEnemy* SpawnMonster = GetWorld()->SpawnActor<AEnemy>(MonsterToSpawn, MonsterSpawnLocation, MonsterSpawnRotation, MonsterSpawnParams);
		UE_LOG(LogTemp, Warning, TEXT("Monster Spawned x %f y %f z %f"), EntityInfo.x, EntityInfo.y, EntityInfo.z);
		UE_LOG(LogTemp, Warning, TEXT("Monster type %d"), EntityInfo.entity_type);

		SpawnMonster->MonsterID = EntityInfo.id;
	}
}

void AWORDSOULPlayerController::SendPlayerInfo()
{
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
}

void AWORDSOULPlayerController::RecvEntitynfo(const SCNewEntity& EntityInfo)
{
	NewEntityInfo = EntityInfo;
}

void AWORDSOULPlayerController::RecvCharacterInfo(const SCPosition& CharacterInfo)
{
	OtherCharacterInfo = CharacterInfo;
}

void AWORDSOULPlayerController::RecvMonsterInfo(const SCPosition& MonsterInfo)
{
	EnemyInfo = MonsterInfo;
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

void AWORDSOULPlayerController::UpdateMonsterInfo(const SCPosition& MonsterInfo)
{
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* cPawn = *It;
		AEnemy* cEnemy = Cast<AEnemy>(cPawn);
		if (cEnemy and cEnemy->MonsterID == MonsterInfo.id)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Monster x y z : %f %f %f"), MonsterInfo.x, MonsterInfo.y, MonsterInfo.z);
			FVector NewLocation = FVector(MonsterInfo.x, MonsterInfo.y, MonsterInfo.z);

			cEnemy->SetActorLocation(NewLocation);
			cEnemy->SetActorRotation(FRotator(0.f, MonsterInfo.r, 0.f));


			UAnimInstance* AnimInst = cEnemy->GetMesh()->GetAnimInstance();
			UWORDSOULAnimInstance* WORDSOULAnimInst = Cast<UWORDSOULAnimInstance>(AnimInst);
			if (WORDSOULAnimInst)
			{
				switch (MonsterInfo.flag & 0b0000'0011)
				{
				case 0: // AI off
					break;
				case 1: // wander
					break;
				case 2: // chase
					break;
				case 3: // attack mode
					break;
				default:
					WORDSOULAnimInst->GroundSpeed = MonsterInfo.v;
					break;
				}

				if ((MonsterInfo.flag & 0b0000'0100) != 0) // attack action
				{
					cEnemy->PlayAttackMontage();
				}
				else if ((MonsterInfo.flag & 0b0000'1000) != 0) // hit reaction
				{

				}
				
			}
			break;
		}
	}
}
