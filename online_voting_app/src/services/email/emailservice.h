
#ifndef EMAILSERVICE_H
#define EMAILSERVICE_H

#include <QFile>
#include <QString>
#include "SmtpMime"

    class EmailService
{
private:
    QString m_smtpHost;
    int m_smtpPort;
    QString m_senderEmail;
    QString m_appPassword;
    bool m_isConfigured;
    EmailService();
    ~EmailService();
    


public:
    EmailService(const EmailService &) = delete;
    void operator=(const EmailService &) = delete;

    static EmailService &getInstance();

    void configure(const QString& host, int port, const QString& email, const QString& password);

   
    bool sendEmail(const QString &recipientEmail,
                   const QString &subject,
                   const QString &bodyContent,
                   bool isHtml = false,
                   const QString &attachmentPath = "");
};

#endif 

