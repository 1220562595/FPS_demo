#pragma once

UENUM(BlueprintType)
enum class ETurning : uint8
{
	ET_Left UMETA(DisplayName = "Turning Left"),
	ET_Right UMETA(DisplayName = "Turning Right"),
	ET_NotTurning UMETA(DisplayName = "Not Turning"),

	ET_Max UMETA(DisplayName = "DefaultMAX")
};