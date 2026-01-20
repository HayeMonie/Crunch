// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemTreeWidget.h"

#include "SplineWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UItemTreeWidget::DrawFromNode(const ITreeNodeInterface* NodeInterface)
{
	if (!NodeInterface)
	{
		return;
	}

	if (CurrentCenterItem == NodeInterface->GetItemObject())
	{
		return;
	}

	ClearTree();
	CurrentCenterItem = NodeInterface->GetItemObject();

	float NextLeafXPosition = 0.f;
	UCanvasPanelSlot* CenterWidgetPanelSlot = nullptr;
	UUserWidget* CenterWidget = CreateWidgetForNode(NodeInterface, CenterWidgetPanelSlot);
	TArray<UCanvasPanelSlot*> UpperStreamSlots;
	TArray<UCanvasPanelSlot*> LowerStreamSlots;
	
	DrawStream(false, NodeInterface, CenterWidget, CenterWidgetPanelSlot, 0, NextLeafXPosition, LowerStreamSlots);
	float LowerStreamXMax = NextLeafXPosition - NodeSize.X - NodeGap.X;
	float LowerMoveAmount = 0.f - LowerStreamXMax / 2.f;
	for (UCanvasPanelSlot* StreamSlot : LowerStreamSlots)
	{
		StreamSlot->SetPosition(StreamSlot->GetPosition() + FVector2D(LowerMoveAmount, 0.f));
	}

	NextLeafXPosition = 0.f;
	
	DrawStream(true, NodeInterface, CenterWidget, CenterWidgetPanelSlot, 0, NextLeafXPosition, UpperStreamSlots);
	float UpperStreamXMax = NextLeafXPosition - NodeSize.X - NodeGap.X;
	float UpperMoveAmount = 0.f - UpperStreamXMax / 2.f;
	for (UCanvasPanelSlot* StreamSlot : UpperStreamSlots)
	{
		StreamSlot->SetPosition(StreamSlot->GetPosition() + FVector2D(UpperMoveAmount, 0.f));
	}

	CenterWidgetPanelSlot->SetPosition(FVector2D::Zero());
}

void UItemTreeWidget::DrawStream(bool bUpperStream, const ITreeNodeInterface* StartingNodeInterface, UUserWidget* StartingNodeWidget, UCanvasPanelSlot* StartingNodeSlot, int StartingNodeDepth, float& NextLeafXPosition, TArray<UCanvasPanelSlot*>& OutStreamSlots)
{
	TArray<const ITreeNodeInterface*> NextTreeNodeInterfaces = bUpperStream ? StartingNodeInterface->GetInputs() : StartingNodeInterface->GetOutputs();

	float StartingNodeYPosition = (NodeSize.Y + NodeGap.Y) * StartingNodeDepth * (bUpperStream ? -1 : 1);
	if (NextTreeNodeInterfaces.Num() == 0)
	{
		StartingNodeSlot->SetPosition(FVector2D(NextLeafXPosition, StartingNodeYPosition));
		NextLeafXPosition += NodeSize.X + NodeGap.X;
		return;
	}

	float NextNodeXPositionSum = 0.f;
	for (const ITreeNodeInterface* NextTreeNodeInterface : NextTreeNodeInterfaces)
	{
		UCanvasPanelSlot* NextWidgetSlot = nullptr;
		UUserWidget* NextWidget = CreateWidgetForNode(NextTreeNodeInterface, NextWidgetSlot);
		OutStreamSlots.Add(NextWidgetSlot);
		
		if (bUpperStream)
		{
			CreateConnection(NextWidget, StartingNodeWidget);
		}
		else
		{
			CreateConnection(StartingNodeWidget, NextWidget);
		}

		DrawStream(bUpperStream, NextTreeNodeInterface, NextWidget, NextWidgetSlot, StartingNodeDepth + 1, NextLeafXPosition, OutStreamSlots);
		NextNodeXPositionSum += NextWidgetSlot->GetPosition().X;
	}

	float StartingNodeXPosition = NextNodeXPositionSum / NextTreeNodeInterfaces.Num();
	StartingNodeSlot->SetPosition(FVector2D(StartingNodeXPosition, StartingNodeYPosition));
}

void UItemTreeWidget::ClearTree()
{
	RootPanel->ClearChildren();
}

UUserWidget* UItemTreeWidget::CreateWidgetForNode(const ITreeNodeInterface* Node, class UCanvasPanelSlot*& OutCanvasSlot)
{
	if (!Node)
	{
		return nullptr;
	}

	UUserWidget* NodeWidget = Node->GetWidget();
	OutCanvasSlot = RootPanel->AddChildToCanvas(NodeWidget);
	if (OutCanvasSlot)
	{
		OutCanvasSlot->SetSize(NodeSize);
		OutCanvasSlot->SetAnchors(FAnchors(0.5f));
		OutCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		OutCanvasSlot->SetZOrder(1);
	}

	return NodeWidget;
}

void UItemTreeWidget::CreateConnection(const UUserWidget* From, const UUserWidget* To)
{
	if (!From || !To)
	{
		return;
	}

	USplineWidget* Connection = CreateWidget<USplineWidget>(GetOwningPlayer());
	UCanvasPanelSlot* ConnectionPanelSlot = RootPanel->AddChildToCanvas(Connection);
	if (ConnectionPanelSlot)
	{
		ConnectionPanelSlot->SetAnchors(FAnchors(0.f));
		ConnectionPanelSlot->SetAlignment(FVector2D(0.f, 0.f));
		ConnectionPanelSlot->SetPosition(FVector2D(0.f, 0.f));
		ConnectionPanelSlot->SetZOrder(0);
	}

	Connection->SetupSpline(From, To, SourcePortLocalPosition, DestinationPortLocalPosition, SourcePortDirection, DestinationPortDirection);
	Connection->SetSplineStyle(ConnectionColor, ConnectionThickness);
}
