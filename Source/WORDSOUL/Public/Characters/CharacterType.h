#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedWeapon UMETA(DisplayName = "EquippedWeapon")
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_Dodge UMETA(DisplayName = "Dodge"),
	EAS_Pickup UMETA(DisplayName = "Pickup")
};

UENUM(BlueprintType)
enum class EMonsterState : uint8
{
	EMS_Wandering UMETA(DisplayName = "Wandering"),
	EMS_Chasing UMETA(DisplayName = "Chasing"),
	EMS_Attacking UMETA(DisplayName = "Attacking"),
	EMS_AIStop UMETA(DisplayName = "AIStop")
};