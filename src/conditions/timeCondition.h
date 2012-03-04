#pragma once
#include "determinatoritems.h"
#include <ctime>

class timeCondition: public DeterminatorCondition<timeCondition>
{
    public:
        timeCondition():mTestTime(0) {}
        timeCondition(const BaseDeterminatorItem::attribute_t& attrlist, std::time_t fixTime);
        timeCondition(const pugi::xml_node& basenode);

        virtual bool match() const;

    private:
        void setCompareTime();
    
        std::time_t mTestTime;
        int mCompareHour;
        int mCompareMinute;
};

