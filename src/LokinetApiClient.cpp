#include "LokinetApiClient.hpp"
#include "lmq_settings.hpp"
#include <QJsonDocument.h>
#include <stdexcept>
#include <cstdio>
#include <QDebug>

bool LokinetApiClient::invoke(const std::string& endpoint, QJsonObject args, ReplyCallback callback) {
  std::cout << "call " << endpoint;
  QJsonDocument doc(args);
  const auto req = doc.toJson();
  if(not lmq_conn.has_value())
  {
    return false;
  }
  lmq.request(
    *lmq_conn,
    std::string_view{endpoint},
    [cb = callback](bool success, std::vector<std::string> data)
    {
      if(success and not data.empty())
      {
        cb(std::move(data[0]));
      }
      else
      {
        cb(std::nullopt);
      }
    }, req.constData());
    return true;
}

Q_INVOKABLE bool LokinetApiClient::invoke(const std::string& endpoint, QJsonObject callargs, QScriptValue callback) {

    if (callback.isValid() && ! callback.isFunction()) {
      qWarning() << "callback should be a function (if present)";
        return false;
    }
    return invoke(endpoint, callargs, [=](std::optional<std::string> reply) mutable {
        QScriptValueList args;
        if(reply.has_value())
        {
          args << QScriptValue(reply->c_str());
          args << QScriptValue(false);
        }
        else
        {
          args << QScriptValue(false);
          args << QScriptValue("no response given from lokinet");
        }
        emit CallCallback(callback, args);
    });
}

LokinetApiClient::LokinetApiClient() : QObject(nullptr)
{
  connect(this, SIGNAL(CallCallback(QScriptValue, QScriptValueList)), this, SLOT(handleCallCallback(QScriptValue, QScriptValueList)));
}

void LokinetApiClient::handleCallCallback(QScriptValue callback, QScriptValueList args) {
  callback.call(QScriptValue(), args);
}
