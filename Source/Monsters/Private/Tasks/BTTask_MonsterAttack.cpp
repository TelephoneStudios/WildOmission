// Copyright Telephone Studios. All Rights Reserved.


#include "Tasks/BTTask_MonsterAttack.h"
#include "Monsters/Monster.h"
#include "AIController.h"

UBTTask_MonsterAttack::UBTTask_MonsterAttack()
{
	NodeName = TEXT("Monster Attack");
}

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* OwnerController = OwnerComp.GetAIOwner();
	if (OwnerController == nullptr)
	{
		return EBTNodeResult::Type::Failed;
	}

	AMonster* OwnerMonster = Cast<AMonster>(OwnerController->GetPawn());
	if (OwnerMonster == nullptr)
	{
		return EBTNodeResult::Type::Failed;
	}

	OwnerMonster->Attack();

	return EBTNodeResult::Type::Succeeded;
}
