// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplineWidget.generated.h"

/**
 * 
 */
UCLASS()
class USplineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupSpline(const UUserWidget* InStartWidget, const UUserWidget* InEndWidget, const FVector2D& InStartPortLocalCoordinate, const FVector2D& InEndPortLocalCoordinate,
		const FVector2D& InStartPortDirection, const FVector2D& InEndPortDirection);

	void SetSplineStyle(const FLinearColor& InColor, float InThickness);

private:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	
	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D TestStartPos;

	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D TestEndPos {FVector2D{100.f, 100.f}};
	
	UPROPERTY(EditAnywhere, Category = "Spline")
	FLinearColor Color {FLinearColor::White};
	
	UPROPERTY(EditAnywhere, Category = "Spline")
	float Thickness {3.0f};
	
	UPROPERTY()
	const UUserWidget* StartWidget;

	UPROPERTY()
	const UUserWidget* EndWidget;

	FVector2D StartPortLocalDirection;
	FVector2D EndPortLocalDirection;

	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D StartPortLocalCoordinate;

	UPROPERTY(EditAnywhere, Category = "Spline")
	FVector2D EndPortLocalCoordinate;
	

};
