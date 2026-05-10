#include "services/email/emailservice.h"
#include <QDebug>
#include <QFile>
#include "../smtp_lib/mimeattachment.h"
#include "../smtp_lib/mimehtml.h"
#include "../smtp_lib/mimemessage.h"
#include "../smtp_lib/mimetext.h"
#include "../smtp_lib/smtpclient.h"

EmailService::EmailService() : m_isConfigured(false) {}
EmailService::~EmailService() {}

void EmailService::configure(const QString &host, int port, const QString &email, const QString &password)
{
    m_smtpHost = host;
    m_smtpPort = port;
    m_senderEmail = email;
    m_appPassword = password;
    m_isConfigured = true;
}

EmailService &EmailService::getInstance()
{
    static EmailService instance;
    return instance;
}

bool EmailService::sendEmail(const QString &recipientEmail,
                             const QString &subject,
                             const QString &bodyContent,
                             bool isHtml,
                             const QString &attachmentPath)
{
    
    SmtpClient smtp(m_smtpHost, m_smtpPort, SmtpClient::SslConnection);

    
    MimeMessage message;
    EmailAddress sender(m_senderEmail, "Election Commission System");
    message.setSender(sender);

    EmailAddress to(recipientEmail, "Voter");
    message.addRecipient(to, MimeMessage::To);
    message.setSubject(subject);

    
    if (isHtml)
    {
        MimeHtml *html = new MimeHtml();
        html->setHtml(bodyContent);
        message.addPart(html, true); 
    }
    else
    {
        MimeText *text = new MimeText();
        text->setText(bodyContent);
        message.addPart(text, true);
    }

    
    if (!attachmentPath.isEmpty())
    {
        QFile *file = new QFile(attachmentPath);
        if (file->exists())
        {
            MimeAttachment *attachment = new MimeAttachment(file);
            message.addPart(attachment, true);
        }
        else
        {
            qDebug() << "Warning: Attachment file not found!";
            delete file;
        }
    }

    
    smtp.connectToHost();
    if (!smtp.waitForReadyConnected(5000))
    { 
        qDebug() << "EmailService Error: Failed to connect to host!";
        return false;
    }

    
    smtp.login(m_senderEmail, m_appPassword, SmtpClient::AuthLogin);
    if (!smtp.waitForAuthenticated(5000))
    {
        qDebug() << "EmailService Error: Authentication failed! Check App Password.";
        smtp.quit();
        return false;
    }

    
    smtp.sendMail(message);
    if (!smtp.waitForMailSent(10000))
    { 
        qDebug() << "EmailService Error: Failed to send the email payload!";
        smtp.quit();
        return false;
    }

    smtp.quit();
    qDebug() << "Success! Email reliably sent to" << recipientEmail;
    return true;
}

