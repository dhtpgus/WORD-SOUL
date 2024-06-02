// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WORDSOULPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/WORDSOULAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AttributeComponent.h"
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
	UpdateMonsterInfo(EnemyInfoMap);
	UpdateEntityHp(HPInfo);
}

void AWORDSOULPlayerController::SpawnMonster(const SCNewEntity& EntityInfo)
{
	if (EntityInfo.entity_type == 2) // spawn monster
	{
		bool bAlreadyExists = false;
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			APawn* cPawn = *It;
			AEnemy* cEnemy = Cast<AEnemy>(cPawn);
			if (cEnemy and cEnemy->MonsterID == EntityInfo.id)
			{
				bAlreadyExists = true;
				break;
			}
		}

		if (!bAlreadyExists)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawned x %f y %f z %f"), EntityInfo.x, EntityInfo.y, EntityInfo.z);
			FVector MonsterSpawnLocation = FVector(EntityInfo.x, -EntityInfo.y, 71.f);
			FRotator MonsterSpawnRotation = FRotator::ZeroRotator;

			FActorSpawnParameters MonsterSpawnParams;
			MonsterSpawnParams.Owner = this;
			MonsterSpawnParams.Instigator = GetInstigator();
			MonsterSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AEnemy* SpawnMonster = GetWorld()->SpawnActor<AEnemy>(MonsterToSpawn, MonsterSpawnLocation, MonsterSpawnRotation, MonsterSpawnParams);


			if (SpawnMonster and SpawnMonster->MonsterID == NULL)
			{
				SpawnMonster->MonsterID = EntityInfo.id;
				SpawnMonster->Attributes->SetMaxHealth(EntityInfo.hp);
				SpawnMonster->Attributes->SetHealth(EntityInfo.hp);
			}
		}
	}
}

void AWORDSOULPlayerController::SendPlayerInfo()
{
	AWORDSOULCharacter* MyCharacter = Cast<AWORDSOULCharacter>(this->GetPawn());
	if (!MyCharacter) return;

	FVector MyLoc = MyCharacter->GetActorLocation();
	MyLoc.Y = -MyLoc.Y;
	float GroundSpeed = MyCharacter->GetGroundSpeed();
	char flag = NULL;
	float MyRot = FMath::DegreesToRadians(MyCharacter->GetActorRotation().Yaw);
	if (MyCharacter->GetIsFalling())
	{
		flag = 0b0000'0001;
	}
	if ((MyCharacter->GetActionState()) == EActionState::EAS_Dodge)
	{
		flag |= 0b000'0100;
	}
	else if (MyCharacter->GetActionState() == EActionState::EAS_Attacking)
	{
		flag |= 0b0000'1000;
	}
	else if (MyCharacter->GetActionState() == EActionState::EAS_Pickup)
	{
		flag |= 0b0100'0000;
	}
	else if (MyCharacter->GetCharacterState() == ECharacterState::ECS_EquippedWeapon)
	{
		flag |= 0b1000'0000;
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
	if (EnemyInfoMap.Contains(MonsterInfo.id))
	{
		EnemyInfoMap[MonsterInfo.id] = MonsterInfo;
	}
	else
	{
		EnemyInfoMap.Add(MonsterInfo.id, MonsterInfo);
	}
}

void AWORDSOULPlayerController::RecvHpInfo(const SCModifyHP& HpInfo)
{
	HPInfo = HpInfo;
}

void AWORDSOULPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UWORDSOULGameInstance* GameInstance = Cast<UWORDSOULGameInstance>(GetWorld()->GetGameInstance());
	if (!GameInstance) return;
	const char* ServerIPChars = TCHAR_TO_ANSI(*GameInstance->IPAddress);

	Socket = ClientSocket::GetInstance();
	Socket->InitSocket();
	bIsConnected = Socket->ConnectToServer(ServerIPChars, SERVER_PORT);
	UE_LOG(LogTemp, Warning, TEXT("%s"), ServerIPChars);
	if (bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER CONNECT SUCCESS"));
		Socket->SetPlayerController(this);

		Socket->Party();
		Socket->StartRecvThread();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SERVER CONNECT FAILED"));
	}

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
			FVector NewLocation = FVector(CharacterInfo.x, -CharacterInfo.y, CharacterInfo.z);

			cCharacter->SetActorLocation(NewLocation);
			cCharacter->SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(CharacterInfo.r), 0.f));

			
			UAnimInstance* AnimInst = cCharacter->GetMesh()->GetAnimInstance();
			UWORDSOULAnimInstance* WORDSOULAnimInst = Cast<UWORDSOULAnimInstance>(AnimInst);
			if (WORDSOULAnimInst)
			{
				WORDSOULAnimInst->GroundSpeed = CharacterInfo.v;
				if ((CharacterInfo.flag & 0b0000'0011) == 0b01) //jumping(IsFalling)
				{
					WORDSOULAnimInst->IsFalling = true;
					//UE_LOG(LogTemp, Warning, TEXT("JUMP : TRUE"));
				}
				else
				{
					WORDSOULAnimInst->IsFalling = false;
				}

				if ((CharacterInfo.flag & 0b0000'0100) != 0)  // dodge
				{
					cCharacter->PlayDodgeMontage();
				}
				else if ((CharacterInfo.flag & 0b0000'1000) != 0)
				{
					cCharacter->PlayAttackMontage();
				}
				else if ((CharacterInfo.flag & 0b0100'0000) != 0)
				{
					cCharacter->PlayPickupMontage();
					AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
					if (OverlappingWeapon)
					{
						OverlappingWeapon->Equip(cCharacter->GetMesh(), FName("RightHandSocket"), cCharacter, cCharacter);
						OverlappingItem = nullptr;
					}
				}
				else if ((CharacterInfo.flag & 0b1000'0000) != 0)
				{
					cCharacter->SetCharacterState(ECharacterState::ECS_EquippedWeapon);
				}
			}
			break;
		}
	}
}

void AWORDSOULPlayerController::UpdateMonsterInfo(const TMap<uint16, SCPosition>& cEnemyInfoMap)
{
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* cPawn = *It;
		AEnemy* cEnemy = Cast<AEnemy>(cPawn);
		if (cEnemy and cEnemyInfoMap.Contains(cEnemy->MonsterID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Monster x y z : %f %f %f"), cEnemyInfoMap[cEnemy->MonsterID].x, cEnemyInfoMap[cEnemy->MonsterID].y, 71.f);
			FVector NewLocation = FVector(cEnemyInfoMap[cEnemy->MonsterID].x, -cEnemyInfoMap[cEnemy->MonsterID].y, 71.f);

			cEnemy->SetActorLocation(NewLocation);
			cEnemy->SetActorRotation(FRotator(0.f, (FMath::RadiansToDegrees(cEnemyInfoMap[cEnemy->MonsterID].r) * -1.0f), 0.f));

			switch (cEnemyInfoMap[cEnemy->MonsterID].flag & 0b0000'0011)
			{
			case 0: // AI off
				cEnemy->MonsterState = EMonsterState::EMS_AIStop;
				break;
			case 1: // wander
				cEnemy->MonsterState = EMonsterState::EMS_Wandering;
				break;
			case 2: // chase
				cEnemy->MonsterState = EMonsterState::EMS_Chasing;
				UE_LOG(LogTemp, Warning, TEXT("chasingState"));
				break;
			case 3: // attack mode
				cEnemy->MonsterState = EMonsterState::EMS_Attacking;
				UE_LOG(LogTemp, Warning, TEXT("AttackingState"));
				break;
			default:
				break;
			}

			if ((cEnemyInfoMap[cEnemy->MonsterID].flag & 0b0000'0100) != 0) // attack action
			{
				cEnemy->PlayAttackMontage();
				UE_LOG(LogTemp, Warning, TEXT("Attack"));
			}
			else if ((cEnemyInfoMap[cEnemy->MonsterID].flag & 0b0000'1000) != 0) // hit reaction (front)
			{
				cEnemy->PlayHitReactMontage("Front");
			}
			else if ((cEnemyInfoMap[cEnemy->MonsterID].flag & 0b0001'0000) != 0) // hit reaction (back)
			{
				cEnemy->PlayHitReactMontage("Back");
			}
		}
	}
}

void AWORDSOULPlayerController::UpdateEntityHp(SCModifyHP& hpInfo)
{
	if (hpInfo.hp != NULL)
	{
		for (TActorIterator<APawn> It(GetWorld()); It; ++It)
		{
			APawn* cPawn = *It;
			AEnemy* cEnemy = Cast<AEnemy>(cPawn);
			AWORDSOULCharacter* cCharacter = Cast<AWORDSOULCharacter>(cPawn);
			if (cEnemy and cEnemy->MonsterID == hpInfo.id)
			{
				float currentHealth = cEnemy->Attributes->GetHealth();
				cEnemy->Attributes->SetHealth(currentHealth + hpInfo.hp);
			}
			if (cCharacter and cCharacter == OtherCharacter)
			{
				float currentHealth = OtherCharacter->Attributes->GetHealth();
				cCharacter->Attributes->SetHealth(currentHealth + hpInfo.hp);
			}
		}
		hpInfo.hp = NULL;
	}

}
