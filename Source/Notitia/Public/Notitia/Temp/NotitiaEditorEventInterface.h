#pragma once

#include "Modules/ModuleInterface.h"

class INotitiaEditorEventInterface
{
public:
	virtual void OnStartupModule() {};
	virtual void OnShutdownModule() {};
};

class INotitiaEditorInterface : public IModuleInterface
{
protected:
	TArray<TSharedRef<INotitiaEditorEventInterface>> ModuleListeners;

public:
	virtual void StartupModule() override
	{
		if (!IsRunningCommandlet())
		{
			AddModuleListeners();
			
			for (TSharedRef<INotitiaEditorEventInterface>& Listener : ModuleListeners)
			{
				Listener->OnStartupModule();
			}
		}
	}

	virtual void ShutdownModule() override
	{
		for (TSharedRef<INotitiaEditorEventInterface>& Listener : ModuleListeners)
		{
			Listener->OnShutdownModule();
		}
	} 

	virtual void AddModuleListeners() {};
};