#include "Controller.h"
#include "ControllersHoarder.h"


namespace eye
{


    ControllersHoarder::ControllersHoarder(unsigned waitEvery):
        m_srgm("srgm"), 
        m_ft5p("ft5p"),
        m_timeout(waitEvery)
    {
    }


    ControllersHoarder::~ControllersHoarder()
    {
    }


    bool ControllersHoarder::update_all()
    {
        bool has_update_srgm = false;
        bool has_update_ft5p = false;
        if (m_timeout)
        {
            m_srgm.update(m_timeout, has_update_srgm);
            m_ft5p.update(m_timeout, has_update_ft5p);
        } 
        else
        {
            m_srgm.update(has_update_srgm);
            m_ft5p.update(has_update_ft5p);
        }
        return has_update_srgm || has_update_ft5p;
    }


    Controller const & ControllersHoarder::srgm() const
    {
        return m_srgm;
    }

    Controller const & ControllersHoarder::ft5p() const
    {
        return m_ft5p;
    }

    Controller & ControllersHoarder::srgm()
    {
        return m_srgm;
    }

    Controller & ControllersHoarder::ft5p()
    {
        return m_ft5p;
    }

}//namespace eye
