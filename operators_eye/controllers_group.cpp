#include "controller.h"
#include "controllers_group.h"


namespace eye
{


    controllers_group::controllers_group(unsigned _wait_every, QObject * _parent):
        QObject(_parent),
        m_srgm(new controller("srgm", this)), 
        m_ft5p(new controller("ft5p", this)),
        m_timeout(_wait_every)
    {
        connect(m_srgm, &controller::update_success, this, &controllers_group::update_completed);
        connect(m_ft5p, &controller::update_success, this, &controllers_group::update_completed);
    }


    controllers_group::~controllers_group()
    {
    }


    void controllers_group::update_all()
    {
        m_changed = false;
        if (m_timeout)
        {
            m_srgm->update(m_timeout);
            m_ft5p->update(m_timeout);
        } 
        else
        {
            m_srgm->update();
            m_ft5p->update();
        }
        if (m_changed)
        {
            emit changed();
        }
    }


    controller & controllers_group::get_srgm() const
    {
        return *m_srgm;
    }

    controller & controllers_group::get_ft5p() const
    {
        return *m_ft5p;
    }


    void controllers_group::update_completed(bool _has_change)
    {
        if (_has_change)
        {
            m_changed = true;
        }
    }


}//namespace eye