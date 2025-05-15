// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARKPROJECT_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	class UButton* ButtonRestart;
	UPROPERTY(meta=(BindWidget))
	class UButton* ButtonQuit;

	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnClickRestart();

	UFUNCTION()
	void OnClickQuit();
};
