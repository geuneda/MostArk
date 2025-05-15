// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHPWidget.h"

#include "Components/ProgressBar.h"

void UPlayerHPWidget::SetHPPercent(float cur, float max)
{
	HPBar->SetPercent(cur/max);
}
