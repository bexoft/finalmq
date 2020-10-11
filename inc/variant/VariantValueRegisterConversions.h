#pragma once

class VariantValueRegisterConversions
{
public:
    VariantValueRegisterConversions();

private:
    template<class TO>
    void registerConversionsTo();
};
