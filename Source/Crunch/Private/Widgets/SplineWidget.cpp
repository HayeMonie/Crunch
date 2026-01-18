// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SplineWidget.h"

void USplineWidget::SetupSpline(const UUserWidget* InStartWidget, const UUserWidget* InEndWidget,
	const FVector2D& InStartPortLocalCoordinate, const FVector2D& InEndPortLocalCoordinate,
	const FVector2D& InStartPortDirection, const FVector2D& InEndPortDirection)
{
	StartWidget = InStartWidget;
	EndWidget = InEndWidget;
	StartPortLocalCoordinate = InStartPortLocalCoordinate;
	EndPortLocalCoordinate = InEndPortLocalCoordinate;
	StartPortLocalDirection = InStartPortDirection;
	EndPortLocalDirection = InEndPortDirection;
}

void USplineWidget::SetSplineStyle(const FLinearColor& InColor, float InThickness)
{
	Color = InColor;
	Thickness = InThickness;
}

int32 USplineWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	FVector2D StartPos = TestStartPos;
	FVector2D EndPos = TestEndPos;

	if (StartWidget && EndWidget)
	{
		StartPos = StartWidget->GetCachedGeometry().GetLocalPositionAtCoordinates(StartPortLocalCoordinate);
		EndPos = EndWidget->GetCachedGeometry().GetLocalPositionAtCoordinates(EndPortLocalCoordinate);
	}

	FSlateDrawElement::MakeSpline(OutDrawElements, ++LayerId, AllottedGeometry.ToPaintGeometry(), StartPos, StartPortLocalDirection, EndPos, EndPortLocalDirection, Thickness, ESlateDrawEffect::None, Color);
	
	return LayerId;
}
