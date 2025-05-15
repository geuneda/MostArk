// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerSkillWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARKPROJECT_API UPlayerSkillWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Skill1Text;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Skill2Text;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* Skill3Text;
	
	// 스킬 쿨타임 텍스트 업데이트 함수
	UFUNCTION()
	void UpdateSkillCooldownText(int32 SkillIndex, float RemainingTime);
	
	// 스킬 쿨타임 텍스트 초기화 함수
	UFUNCTION()
	void ResetSkillCooldownText(int32 SkillIndex);
};
