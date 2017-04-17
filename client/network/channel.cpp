#include "channel.h"
#include "tcpclient.h"
#include "tcpclientitem.h"

Channel::Channel()
{

}

Channel::Channel(QString str)
{
    m_name = str;
}

Channel::~Channel()
{

}

QString Channel::password() const
{
    return m_password;
}

void Channel::setPassword(const QString &password)
{
    m_password = password;
}

int Channel::indexOf(TreeItem *child)
{
    return m_child.indexOf(child);
}

QString Channel::description() const
{
    return m_description;
}

void Channel::setDescription(const QString &description)
{
    m_description = description;
}

bool Channel::usersListed() const
{
    return m_usersListed;
}

void Channel::setUsersListed(bool usersListed)
{
    m_usersListed = usersListed;
}

bool Channel::isLeaf() const
{
    return false;
}
void Channel::readChannel(QJsonObject &json)
{
    m_password=json["password"].toString();
    m_name=json["title"].toString();
    m_description=json["description"].toString();
    m_usersListed=json["usersListed"].toBool();

    QJsonArray array = json["channel"].toArray();
    for(auto channelJson : array)
    {
        Channel* chan = new Channel();
        QJsonObject obj = channelJson.toObject();
        chan->readChannel(obj);
        m_child.append(chan);
    }
}

void Channel::writeChannel(QJsonObject &json)
{
    json["password"]=m_password;
    json["title"]=m_name;
    json["description"]=m_description;
    json["usersListed"]=m_usersListed;

    QJsonArray array;
    for (int i = 0; i < m_child.size(); ++i)
    {
        if(m_child.at(i)->isLeaf())
        {
            Channel* item = dynamic_cast<Channel*>(m_child.at(i));
            if(nullptr != item)
            {
                QJsonObject jsonObj;
                item->writeChannel(jsonObj);
                array.append(jsonObj);
            }
        }
    }
    json["channel"]=array;
}
void Channel::sendToAll(NetworkMessageReader* msg, TcpClient* tcp)
{
    m_dataToSend.append(msg);
    for(auto client : m_child)
    {
        TcpClientItem* item = dynamic_cast<TcpClientItem*>(client);

        if(nullptr != item)
        {
            TcpClient* other = item->client();
            if((nullptr != other)&&(other!=tcp))
            {
                qDebug()<< "[server][send to clients]" << other << tcp;
                other->sendMessage(*msg);
            }
        }
    }
}
int Channel::addChild(TreeItem* item)
{
    m_child.append(item);
    TcpClientItem* tcp = dynamic_cast<TcpClientItem*>(item);
    if(nullptr!=tcp)
    {
        tcp->setChannelParent(this);
        TcpClient* newClient = tcp->client();
        if(nullptr != newClient)
        {//resend all previous data received
            for(auto msg : m_dataToSend)
            {
                newClient->sendMessage(*msg);
            }
        }
    }
    return m_child.size();

}
bool Channel::addChildInto(QString id, TreeItem* child)
{
    if(m_id == id)
    {
        addChild(child);
        return true;
    }
    else
    {
        for(TreeItem* item : m_child)
        {
            if(item->addChildInto(id, child))
            {
                return true;
            }
        }
    }
    return false;
}
