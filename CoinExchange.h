#pragma once

#include “CoreMinimal.h”
#include “UI/Base/FormBase.h”
#include “CoinExchange_Form.generated.h”

class UButton;
class UImage;
class UListView;
class UTextBlock;

class Table;

class UListItmeBase;
class UCoinExchange_Tab_ListItem_ItemData;
class UCoinExchange_Item_ListItem_ItemData;

UClass()
Class ProjectAPI UCoinExchange_Form : public UFormBase
{
GENERATED_BODY()

public:
	virtual void InitWidget();

private:
#pragma region init
	void InitTab();
#pragma endregion init

#pragma region main process
	void ChangeTab(const int32 InTabIndex);
	void UpdateExchangeView();
	void RecvServerDataUpdateView();
	void ShowNotice(const int32 InTableID);
#pragma endregion main process

#pragma region setting ui
	void SetNeedCost();
	void SetTargetCostInfo(const int32 InItemTableID);
	void SetTargetAmountText(const int32 InHaveAmount);
#pragma endregion setting ui

#pragma region listview event
	UFUNCTION() void OnListItemObjectSet_Event_ListView_Tab(UListItemBase* InItemBase);
	UFUNCTION() void OnListItemObjectSetEvent_ListView_Exchange(UListItemBase* InItemBase);
#pragma endregion listview event

#pragma region onclick event
	UFUNCTION() void OnClick_AllRefresh();									//	초기화 버튼
#pragma endregion onclick event

#pragma region callback event
	void CallBack_Update();												//	+, - 버튼 인풋
	void CallBack_ClickTab(const int32& InTabIndex);					//	탭 변경 
	void CallBack_Exchange(const int32 InTableID, const int32 InCount);	//	아이템 교환
#pragma endregion callback event

private:
#pragma region bind widget
	UTextBlock* Text_TargetName = nullptr;			// ex) 길드 주화 사용
	UTextBlock* Text_TargetAmount = nullptr;		// 현재 가지고 있는 수
	UTextBlock* Text_AllNeedCost = nullptr;			// - 3,000 전체 얼만큼 빠질지.

	UListView* ListView_CategoryTab = nullptr;
	UListView* ListView_CoinExchange = nullptr;

	UButton* Bt_AllRefresh = nullptr;

	UImage* Icon_TargetCoin = nullptr;
#pragma endregion bind widget

#pragma region listview item data
	UPROPERTY() TMap<int32, UCoinExchange_Tab_ListItem_ItemData*> m_TabList;
	UPROPERTY() TArray<UCoinExchange_Item_ListItem_ItemData*> m_ItemList;
	UPROPERTY() TArray<UCoinExchange_Item_ListItem_ItemData*> m_PoolList;
#pragma endregion listview item data

	UTable* m_Table = nullptr;
	EServerSendType m_ServerSendType;

	int32 m_SelectTabIndex = 0;
	int32 m_SelectItemIndex = 0;
}
