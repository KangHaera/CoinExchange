#include "CoinExchange_Form.h"

#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "Table/Table.h"
#include "Table/ItemMain.h"
#include "Table/CoinExchangeMain.h"
#include "Table/CoinExchangeCategory.h"

#include "FunctionLibrary_System.h"
#include "FunctionLibrary_Network.h"

#include "System/GameUISystem.h"

#include "User/User.h"
#include "User/Item/Item.h"
#include "User/Item/ItemInventory.h"

#include "Network/Server/Requester/ShopRequester.h"

#include "UI/Form/CoinExchange/ListItem/CoinExchange_Tab_ListItem.h"
#include "UI/Form/CoinExchange/ListItem/CoinExchange_Item_ListItem.h"


void CoinExchange_Form::InitWidget()
{
    Table = FunctionLibrary_System::GetTable(this);

    UE_BIND_WIDGET(UListView, ListView_CategoryTab);
    UE_BIND_WIDGET(UListView, ListView_CoinExchange);

    UE_BIND_WIDGET(UImage, Icon_TargetCoin);

    UE_BIND_WIDGET(UTextBlock, Text_TargetName);
    UE_BIND_WIDGET(UTextBlock, Text_TargetAmount );
    UE_BIND_WIDGET(UTextBlock, Text_AllNeedCost);

    UE_BIND_WIDGET(UButton, Bt_AllRefresh);
    if (Bt_AllRefresh == nullptr ||  Bt_AllRefresh->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bt_AllRefresh is nullptr"));
        return;
    }
    Bt_AllRefresh->OnClicked.AddUniqueDynamic(this, &CoinExchange_Form::OnClick_AllRefresh);


    InitTab();
}

void CoinExchange_Form::InitTab()
{
    if (Table == nullptr ||  Table->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Table is nullptr"));
        return;
    }

    TArray<FCoinExchangeCategory> CategoryDataList;
    if (Table->GetAllCoinExchangeCategoryData(CategoryDataList) == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Table->GetAllCoinExchangeCategoryData(CategoryDataList) == false"));
        return;
    }

    if (ListView_CategoryTab == nullptr ||  ListView_CategoryTab->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("ListView_CategoryTab is nullptr"));
        return;
    }
    
    for (int i= 0; i < CategoryDataList.Num(); ++i)
    {
        UCoinExchange_Tab_ListItem_ItemData* CreateData = NewObject<UCoinExchange_Tab_ListItem_ItemData>(this);
        FItemMain* CoinItemData = Table->GetItemMainDataByID(CategoryDataList[i].item_idx);
        if (CoinItemData == nullptr)
        {
            continue;
        }

        if (CoinItemData.icon_path.Get() == nullptr)
        {
            CoinItemData.icon_path.LoadSynchronous();
        }

        CreateData->SetData(IconCode.icon_path.Get(), CoinData->name.ToString(), CategoryData[i].coin_exchange_category);
        CreateData->SetActive(false);
        CreateData->SetOnDataEvent(UListItemBase::FOnSetDataBase::CreateUObejct(this, &CoinExchange_Form::OnListItemObjectSet_Event_ListView_CategoryTab));

        TabList.Emplace(CategoryData[i].coin_exchange_category, CreateData);
        ListView_CategoryTab->AddItem(CreateData);
    }

    ChangeTab(CategoryData[0].coin_exchange_category);
    ListView_CategoryTab->RegenerateAllEntries();
}

void CoinExchange_Form::ChangeTab(const int32 InTabIndex)
{
    if (SelectTabIndex == InTabIndex)
    {
        //  같은 탭 예외처리
        return;
    }

    //  이전 탭 off
    if (TabList.Contains(SelectTabIndex) == true)
    {
        TabList[SelectTabIndex]->IsActive = false;
    }
 
    //  탭 갱신
    SelectTabIndex = InTabIndex;

    if (TabList.Contains(SelectTabIndex) == true)
    {
        TabList[SelectTabIndex]->IsActive = true;
    }

    if (ListView_CategoryTab == nullptr ||  ListView_CategoryTab->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("ListView_CategoryTab is nullptr"));
        return;
    }

    ListView_CategoryTab->RegenerateAllEntries();
	UpdateExchangeView();
}

void CoinExchange_Form::UpdateExchangeView()
{
    //  이전 데이터 저장
    ClearItemListView();

    if (Table == nullptr || Table->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Table is nullptr"));
        return;
    }

	//	테이블 데이터 가져오기
    TArray<FCoinExchangeMain> CoinExchangeItemList;
    if (Table->GetCoinExchangeMainDataListByCategoryID(SelectTabIndex, CoinExchangeItemList) == false)
    {
        UE_LOG(LogTemp, Warning
			, TEXT("Not Found FCoinExchangeMain Table Data.")
			, SelectTabIndex);
        return;
    }

    //  가지고 있는 코인 세팅
    SetTargetCostInfo(CoinExchangeItemList[0].cost_item_idx);
	
	//	가지고 있는 아이템 카운트 세팅
	UUser* User = UFunctionLibrary_System::GetUser(this);
	if (User == nullptr && User->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("User is nullptr"));
		return;
	}

	UItemInventory* Inven = User->GetInventoryByInvenType<UItemInventory>(EInvenType::Item);
	if (Inven == nullptr && Inven->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inven is nullptr"));
		return;
	}

	int32 TargetItemCount = Inven->GetInventoryItemCountByDataID(MainData.cost_item_idx);
	SetTargetAmountText(TargetItemCount);

    if (ListView_CoinExchange == nullptr && ListView_CoinExchange->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("mListView_CoinExchange is nullptr"));
        return;
    }

	//	테이블 데이터만큼 위젯 생성
    for (int i = 0; i < CoinExchangeItemList.Num(); ++i)
    {
        UCoinExchange_Item_ListItem_ItemData* CreateData = nullptr;

        if (PoolItemList.Num() > 0)
        {
            CreateData = PoolItemList.Pop();
        }
        else 
        {
            CreateData = NewObject<UCoinExchange_Item_ListItem_ItemData>(this);
        }

        if (CreateData == nullptr && CreateData->IsValidLowLevel() == false)
        {
            UE_LOG(LogTemp, Warning, TEXT("CreateData is nullptr"));
            continue;
        }

        FItemMain* CoinItemData = Table->GetItemMainDataByID(CoinExchangeItemList[i].item_idx);
        if (CoinItemData == nullptr)
        {
            continue;
        }

        if (CoinItemData.icon_path.Get() == nullptr)
        {
            CoinItemData.icon_path.LoadSynchronous();
        }

        FItemMain* ChangeItemData = Table->GetItemMainDataByID(CoinExchangeItemList[i].change_item_idx);
        if (ChangeItemData == nullptr)
        {
            continue;
        }

        if (ChangeItemData.icon_path.Get() == nullptr)
        {
            ChangeItemData.icon_path.LoadSynchronous();
        }

        CreateData.SetData(CoinExchangeItemList[i].coin_exchange_idx, CoinItemData.icon_path.Get(), CoinExchangeItemList[i].cost_count
            , ChangeItemData.icon_path.Get(),  CoinExchangeItemList[i].change_count);
        CreateData->SetOnDataEvent(UListItemBase::FOnSetDataBase::CreateUObejct(this, &CoinExchange_Form::OnListItemObjectSetEvent_ListView_CoinExchange));

        ItemList.Emplace(CreateData);
        ListView_CoinExchange->AddItem(CreateData);
    }

    ListView_CoinExchange->RegenerateAllEntries();

    SetAllNeedCount();
}

void CoinExchange_Form::RecvServerDataUpdateView()
{
    if (ItemList.Num() <= 0)
    {
		UpdateExchangeView();
        return;
    }

    FCoinExchangeMain MainData;
    if (Table->GetCoinExchangeMainDataAt(ItemList[0]->GetTableID(), MainData) == false)
    {
        UE_LOG(LogTemp, Warning
			, TEXT("Not Found FCoinExchangeMain Table Data. ItemList[0]->GetTableID() index is [%d]")
			, ItemList[0]->GetTableID());
        return;
    }

	//	가지고 있는 아이템 카운트 세팅
	UUser* User = UFunctionLibrary_System::GetUser(this);
	if (User == nullptr && User->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("User is nullptr"));
		return;
	}

	UItemInventory* Inven = User->GetInventoryByInvenType<UItemInventory>(EInvenType::Item);
	if (Inven == nullptr && Inven->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inven is nullptr"));
		return;
	}

	int32 TargetItemCount = Inven->GetInventoryItemCountByDataID(MainData.cost_item_idx);
	SetTargetAmountText(TargetItemCount);

	//	아이템 카운트 재세팅
    int32 ChangeItemHaveCount = Inven->GetInventoryItemCountByDataID(MainData.item_idx);
    for (int i= 0; i < ItemList.Num(); ++i)
    {
        if (ItemList[i]->GetTableID() == SelectItemIndex)
        {
            ItemList[i]->SetCount(0);
            ItemList[i]->SetChangeCoinHaveCount(ChangeItemHaveCount);
			SelectItemIndex = 0;
            break;
        }
    }

    if (ListView_CoinExchange == nullptr && ListView_CoinExchange->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("mListView_CoinExchange is nullptr"));
        return;
    }
    ListView_CoinExchange->RegenerateAllEntries();

    SetNeedCost();
}

void CoinExchange_Form::ShowNotice(const int32 InTableID)
{
    if (Owner == nullptr && Owner->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Owner is nullptr"));
        return;
    }

    FNoticeArgs Args;
    Args.TableID = InTableID;
    Args.NoticeTableType = ENoticeTableType::SystemMsg;

    Ower->ShowNotice(ENoticeWidget::Notice_Larg, &Args);
}

void CoinExchange_Form::SetNeedCost()
{
	if (Text_AllNeedCost == nullptr || Text_AllNeedCost->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Text_AllNeedCost is nullptr"));
		return;
	}

	int32 NeedCost = 0;
	for (int i = 0; i < ItemList.Num(); ++i)
	{
		if (ItemList[i] == nullptr || ItemList[i]->IsValidLowLevel() == false)
		{
			continue;
		}

		NeedCost += ItemList[i]->GetNeedCostCount();
	}

	Text_AllNeedCost->SetText(NeedCost == 0 ? TEXT() : FText::FromString(TEXT("-") + FText::AsNumber(NeedCost).ToString()));
}

void CoinExchange_Form::SetTargetCostInfo(const int32 InItemTableID)
{
	if (Table == nullptr || Table->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Table is nullptr"));
		return;
	}

    FItemMain* CoinItemData = Table->GetItemMainDataByID(InItemTableID);
    if (CoinItemData == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("CoinItemData is nullptr"));
        return;
    }

    if (CoinItemData.icon_path.Get() == nullptr)
    {
        CoinItemData.icon_path.LoadSynchronous();
    }

	if (Icon_TargetCoin == nullptr || Icon_TargetCoin->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Icon_TargetCoin is nullptr"));
		return;
	}

	Icon_TargetCoin->SetBrushResourceObject(CoinItemData.icon_path.Get());

    if (Text_TargetName == nullptr || Text_TargetName->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Text_TargetName is nullptr"));
        return;
    }

    FText HaveText;
    Table->GetUIMSgStringDataAt(EUIMsg::UseAmount, HaveText);	//	사용
    HaveText = FText::FromString(TEXT(" ") + CoinItemData.name.ToString() + HaveText.ToString());
	Text_TargetName->SetText(HaveText);
}

void CoinExchange_Form::SetTargetAmountText(const int32 InHaveAmount)
{
	if (Text_TargetAmount  == nullptr || Text_TargetAmount ->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Text_TargetAmount  is nullptr"));
		return;
	}

    Text_TargetAmount->SetText(FText::AsNumber(InHaveAmount));
}

void CoinExchange_Form::ClearItemListView()
{
    if (ListView_CoinExchange == nullptr || ListView_CoinExchange->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("ListView_CoinExchange is nullptr"));
        return;
    }

    for (int i = 0; i < ListView_CoinExchange->GetNumItems(); ++i)
    {
        PoolItemList.Emplace(Cast<UCoinExchange_Item_ListItem_ItemData>(ListView_CoinExchange->GetItemAt(i)));
    }

    ItemList.Empty();
    ListView_CoinExchange->ClearListItems();
}

void CoinExchange_Form::OnListItemObjectSet_Event_ListView_CategoryTab(UListItemBase* InItemBase)
{
	if (InItemBase == nullptr || InItemBase->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("InItemBase is nullptr"));
		return;
	}

	UCoinExchange_Tab_ListItem* CastItem = Cast<UCoinExchange_Tab_ListItem>(InItemBase);
	if (CastItem == nullptr || CastItem->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CastItem is nullptr"));
		return;
	}

	CastItem->SetOnTabClickEvent(UCoinExchange_Tab_ListItem::FOnClickTab::CreateUObject(this, &CoinExchange_Form::CallBack_ClickTab));
}

void CoinExchange_Form::OnListItemObjectSetEvent_ListView_CoinExchange(UListItemBase* InItemBase)
{
	if (InItemBase == nullptr || InItemBase->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("InItemBase is nullptr"));
		return;
	}

	UCoinExchange_Item_ListItem* CastItem = Cast<UCoinExchange_Item_ListItem>(InItemBase);
	if (CastItem == nullptr || CastItem->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CastItem is nullptr"));
		return;
	}

	CastItem->SetOnClickBuy(UCoinExchange_Item_ListItem::FOnCLickBuyEvent::CreateUObject(this, &CoinExchange_Form::CallBack_Exchange));
	CastItem->SetUpdateEvent(UCoinExchange_Item_ListItem::FOnUpdateEvent::CreateUObject(this, &CoinExchange_Form::CallBack_Update));
}

void UCoinExchange_Form::CallBack_Update()
{
	SetNeedCost();
}

void CoinExchange_Form::OnClick_AllRefresh()
{
    if (ListView_CoinExchange == nullptr || ListView_CoinExchange->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("ListView_CoinExchange is nullptr"));
        return;
    }

    for (int i = 0; i < ItemList.Num(); ++i)
    {
		if (ItemList[i] == nullptr || ItemList[i]->IsValidLowLevel() == false)
		{
			continue;
		}
        ItemList[i]->SetCount(0);
    }

    ListView_CoinExchange->RegenerateAllEntries();
}

void UCoinExchange_Form::CallBack_ClickTab(const int32& InTabIndex)
{
    ChangeTab(InTabIndex);
}

void UCoinExchange_Form::CallBack_Exchange(const int32 InTableID, const int32 InCount)
{
	if (ServerSendType == EServerSendType::Send)
	{
		return;
	}

    if (InTableID == INDEX_NONE)
    {
        ShowNotice(static_cast<int32>(InCount <= 0 ? ESysMsg::SysMsg_ExchangeReceiptInput : ESysMsg::SysMsg_ExchangeInsuffcient));
        return;
    }

	SelectItemIndex = InTableID;

    UShopRequester* Requester = UFunctionLibrary_Network::GetRequester<UShopRequester>();
    if (Requester == nullptr || Requester->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("Requester is nullptr"));
        return;
    }

    UUser* User = UFunctionLibrary_System::GetUser();
     if (User == nullptr || User->IsValidLowLevel() == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("User is nullptr"));
        return;
    }

    Requester->Send_CoinExchangeItem(this, User->GetUserInfo(), User->GetRandKey(), InTableID, InCount);

	ServerSendType = EServerSendType::Send;
}

void UCoinExchange_Form::Recv_CoinExchange_Item()
{
	ServerSendType = EServerSendType::Recv;
	RecvServerDataUpdateView();
    ShowNotice(static_cast<int32>(ESysMsg::SysMsg_TradeComplete));
}
