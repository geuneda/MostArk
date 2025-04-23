#include "ArkProjectGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Player/MostArkPlayer.h"
#include "HUD/TripodSystemHUD.h"

AArkProjectGameMode::AArkProjectGameMode()
{
    // 기본 폰 클래스를 MostArkPlayer로 설정
    DefaultPawnClass = AMostArkPlayer::StaticClass();
    
    // 기본 HUD 클래스를 TripodSystemHUD로 설정
    HUDClass = ATripodSystemHUD::StaticClass();
}