#pragma once
#ifndef INCLUDE_OPERATOR_EYE_CONTROLLERSHOARDER_H
#define INCLUDE_OPERATOR_EYE_CONTROLLERSHOARDER_H
#include <memory>
#include "Controller.h"

namespace eye
{

    class ControllersHoarder
    {
    public:
        ControllersHoarder(unsigned waitEvery);
        ~ControllersHoarder();

        Controller const & srgm() const;
        Controller const & ft5p() const;
        Controller & srgm();
        Controller & ft5p();

        bool update_all();
    private:
        Controller m_srgm;
        Controller m_ft5p;
        unsigned m_timeout;
    };


}//namespace eye


#endif //INCLUDE_OPERATOR_EYE_CONTROLLERSHOARDER_H
