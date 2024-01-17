// Copyright arcrofts2000


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/TargetInterface.h"

AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
	
	LastActor = CurrentActor;
	CurrentActor = Cast<ITargetInterface>(CursorHit.GetActor());

	/**
	* Line trace from cursor. There are several scenarios:
	*   A. LastActor is null && CurrentActor is null
	*       - Do nothing.
	*   B. LastActor is null && CurrentActor is valid
	*	    - Highlight CurrentActor
	*   C. LastActor is valid && CurrentActor is null
	*	    - UnHighlight LastActor
	*   D. Both are valid, but LastActor != CurrentActor
	*		- Highlight CurrentActor
	*		- UnHighlight LastActor
	*	E. Both are valid, and the same actor
	*		- Do nothing.
	*/

	if (LastActor == nullptr)
	{
		if (CurrentActor != nullptr)
		{
			// Case B: Only CurrentActor is valid.
			CurrentActor->HighlightActor();
		}
		else
		{
			// Case A: Both are null. Do nothing.
		}
	}
	else // LastActor is valid.
	{
		if (CurrentActor == nullptr)
		{
			// Case C: No longer hovering over an actor.
			LastActor->UnHighlightActor();
		}
		else // Both actors are valid.
		{
			if (LastActor != CurrentActor)
			{
				// Case D: Both valid, but not the same
				LastActor->UnHighlightActor();
				CurrentActor->HighlightActor();
			}
			else
			{
				// Case E: Both the same. Do nothing.
			}
		}
	}
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);
	Subsystem->AddMappingContext(AuraContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& Value)
{
	const FVector2D InputAxisVector = Value.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}