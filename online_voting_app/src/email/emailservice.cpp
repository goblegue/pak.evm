#include "emailservice.h"
#include <QDebug>
#include <QFile>
#include "../smtp_lib/mimeattachment.h"
#include "../smtp_lib/mimehtml.h"
#include "../smtp_lib/mimemessage.h"
#include "../smtp_lib/mimetext.h"
#include "../smtp_lib/smtpclient.h"

EmailService::EmailService(QString host, int port, QString email, QString password)
{
    m_smtpHost = host;
    m_smtpPort = port;
    m_senderEmail = email;
    m_appPassword = password;
}

bool EmailService::sendEmail(const QString &recipientEmail,
                             const QString &subject,
                             const QString &bodyContent,
                             bool isHtml,
                             const QString &attachmentPath)
{
    // 1. Setup Connection (SslConnection for Port 465)
    SmtpClient smtp(m_smtpHost, m_smtpPort, SmtpClient::SslConnection);

    // 2. Build the Message Headers
    MimeMessage message;
    EmailAddress sender(m_senderEmail, "Election Commission System");
    message.setSender(sender);

    EmailAddress to(recipientEmail, "Voter");
    message.addRecipient(to, MimeMessage::To);
    message.setSubject(subject);

    // 3. Add the Body (Text or HTML)
    if (isHtml) {
        MimeHtml *html = new MimeHtml();
        html->setHtml(bodyContent);
        message.addPart(html, true); // true = take ownership (deletes memory automatically)
    } else {
        MimeText *text = new MimeText();
        text->setText(bodyContent);
        message.addPart(text, true);
    }

    // 4. Handle Attachments (Optional)
    if (!attachmentPath.isEmpty()) {
        QFile *file = new QFile(attachmentPath);
        if (file->exists()) {
            MimeAttachment *attachment = new MimeAttachment(file);
            message.addPart(attachment, true);
        } else {
            qDebug() << "Warning: Attachment file not found!";
            delete file;
        }
    }

    // Step A: Connect to Google
    smtp.connectToHost();
    if (!smtp.waitForReadyConnected(5000)) { // Wait up to 5 seconds
        qDebug() << "EmailService Error: Failed to connect to host!";
        return false;
    }

    // Step B: Authenticate with App Password
    smtp.login(m_senderEmail, m_appPassword, SmtpClient::AuthLogin);
    if (!smtp.waitForAuthenticated(5000)) {
        qDebug() << "EmailService Error: Authentication failed! Check App Password.";
        smtp.quit();
        return false;
    }

    // Step C: Send the Email
    smtp.sendMail(message);
    if (!smtp.waitForMailSent(10000)) { // Wait up to 10 seconds for large attachments
        qDebug() << "EmailService Error: Failed to send the email payload!";
        smtp.quit();
        return false;
    }

    // 6. Close the connection gracefully
    smtp.quit();
    qDebug() << "Success! Email reliably sent to" << recipientEmail;
    return true;
}