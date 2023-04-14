/*
 * File: BaseItem.h
 * Course: GAM300
 * Author(s): Alejandro Ramirez
 *
 * All content © 2020 DigiPen (USA) Corporation, all rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemInterface.h"

#include "BaseItem.generated.h"

UCLASS(BlueprintType)
class GAM300_PROJECT_API ABaseItem : public AActor, public IItemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
		UStaticMeshComponent* meshComp;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Item Property")
	bool is_stackable_ = false;
	
	UPROPERTY(EditAnywhere, Category="Item Property")
	int stack_size_ = 1;

	UPROPERTY(EditAnywhere, Category="Item Property")
	FString description_;

public:	
	// Inherited Interface
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteract(AActor* Caller);
	virtual void OnInteract_Implementation(AActor* Caller);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Use(AActor* Caller);
	virtual void Use_Implementation(AActor* Caller);

	// Getters
	UFUNCTION(BlueprintCallable, Category="Base Item")
	FString GetItemDescription() const;
	
	UFUNCTION(BlueprintCallable, Category="Base Item")
	bool IsStackable() const;
	
	UFUNCTION(BlueprintCallable, Category="Base Item")
	int GetStackSize() const;
};
