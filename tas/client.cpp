#include <Windows.h>
#include "overlapped_server.h"
#include "client.h"


tas_client::tas_client(tas_controller * _ctrls, unsigned _count) noexcept:
    m_controllers(nullptr),
    m_controllers_count(0),
    m_last_update_time(0),
    m_last_error(0),
    m_connected(false)
{
}


tas_client::~tas_client() noexcept
{
    m_pipe.close();
}


bool tas_client::to_work(tas_overlapped_server & _osrv) noexcept
{
    m_last_error = m_pipe.create(sc_pipes_name, sc_pipes_options);
    if (!m_last_error)
    {
        m_last_error = _osrv.connect_named_pipe(
            m_pipe.handle(),
            nullptr,
            0,
            callback_connected,
            this
        );
    }
    return m_last_error == 0;
}


void tas_client::close_channel() noexcept
{
    disconnect();
    m_pipe.close();
}


bool tas_client::reaccept(tas_overlapped_server & _osrv) noexcept
{
    m_pipe.disconnect();
    m_last_error = _osrv.connect_named_pipe(
        m_pipe.handle(),
        nullptr,
        0,
        callback_connected,
        this
    );
    return m_last_error == 0;
}


bool tas_client::connected() const noexcept
{
    return m_connected;
}


tas_size tas_client::last_time() const noexcept
{
    return m_last_update_time;
}


tas_error tas_client::last_error() const noexcept
{
    return m_last_error;
}


tas_size tas_client::process_query(tas_size _query_size) const noexcept
{
/*
* ��������� ������� � �������� ����������:
* name{cdx1 cdx2 ... cdxN} name{cdx1 cdx2 ... cdxN}
* ������������ ����� ������� - 1023 �����
*
* ��������� ������� � �������:
* ready name date {cdx1:value cdx2:value ... cdxN:value} ...
* date - ���� ���������� ����������� ������ � ����������� � ������� YYYY.MM.DD/HH:MM:SS
* value - ��������� ������������� �������� ������� �� 7-�� �������� ������������.
* ��������� �� �������:
* ������������ ��� �����������:
* error name {invalid name} ...
* ������ � ����������� ��� �� ���������:
* error name {no data available} ...
* ���� ��� ��������� ���������� �� ������� ��� ��� �� ��������, ���� cdx ����� ������������ ������
* ready name date {cdx1:value cdx2:error ... cdxN:error} ...
* ������������ �������� ������:
* error {overflow input buffer}
* ������������ ��������� ������:
* ... error {overflow output buffer}
* ������������ ����� ������ - 4096 ����
*
* ������������ ����� ������������ ������������ �������� - sc_max_clients (5)
*/
    tas_size ansfer_size = 0;

    tas_query_parser parser(m_controllers, m_controllers_count);
    ansfer_size = parser.parse(m_in_buffer, _query_size, m_out_buffer, sizeof(m_out_buffer));

    return ansfer_size;
}


bool tas_client::read(tas_overlapped_server & _osrv) noexcept
{
    m_last_error = _osrv.read_file(
        m_pipe.handle(),
        m_in_buffer,
        sizeof(m_in_buffer) - 1,
        callback_read,
        this
    );
    return m_last_error == 0;
}


bool tas_client::write(tas_overlapped_server & _osrv, tas_size _count) noexcept
{
    m_last_error = _osrv.write_file(
        m_pipe.handle(),
        m_out_buffer,
        _count,
        callback_write,
        this
    );
    return m_last_error == 0;
}


void tas_client::disconnect() noexcept
{
    m_connected = false;
    m_pipe.disconnect();
}


void tas_client::callback_connected(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e)
{
    tas_client * client = static_cast<tas_client *>(_operation.exdata);
    client->m_last_update_time = GetTickCount();
    if (_e)
    {
        //todo: log debug
        client->reaccept(_srv);
        //todo: log error
        return;
    }

    if (client->read(_srv))
    {
        client->m_connected = true;
        return;
    }
    //todo: log error
    client->reaccept(_srv);
    //todo: log error
}


void tas_client::callback_read(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e)
{
    tas_client * client = static_cast<tas_client *>(_operation.exdata);
    client->m_last_update_time = GetTickCount();
    if (_e)
    {
        client->reaccept(_srv);
        return;
    }
    
    tas_size ansfer_size = client->process_query(_operation.written);

    if (ansfer_size == 0 || !client->write(_srv, ansfer_size))
    {
        client->reaccept(_srv);
    }    
}


void tas_client::callback_write(tas_overlapped_server & _srv, tas_operation const & _operation, tas_error _e)
{
    tas_client * client = static_cast<tas_client *>(_operation.exdata);
    client->m_last_update_time = GetTickCount();
    client->reaccept(_srv);
}

