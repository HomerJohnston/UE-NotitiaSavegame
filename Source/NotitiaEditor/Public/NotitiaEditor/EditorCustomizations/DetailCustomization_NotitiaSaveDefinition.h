﻿#pragma once

#include "IDetailCustomization.h"

class FDetailCustomization_NotitiaSaveDefinition : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
