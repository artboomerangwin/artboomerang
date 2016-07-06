/*
Copyright (c) 2013 Raivis Strogonovs

http://morf.lv

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/



#include "smtp.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QSslConfiguration>

Smtp::Smtp( const QString &user, const QString &pass, const QString &host, int port, int timeout )
{    

    this->appTitle = tr( "SMTP client" );

    this->socket = new QSslSocket(this);
    this->cc.clear();

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected() ) );
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(errorReceived(QAbstractSocket::SocketError)));   
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(socket, SIGNAL(disconnected()), this,SLOT(disconnected()));
    connect(socket, SIGNAL(disconnected()),this,SLOT(deleteLater()));


    this->user = user;
    this->pass = pass;

    this->host = host;
    this->port = port;
    this->timeout = timeout;


}


void Smtp::setModel(OptionsModel *model)
{
    if(model && model->getProxyUse()){
        //QNetworkProxy proxy = QNetworkProxy(QNetworkProxy::Socks5Proxy, "localhost", 9500);
        QNetworkProxy proxy = QNetworkProxy(
                    model->getProxySocks()==5? QNetworkProxy::Socks5Proxy :  QNetworkProxy::DefaultProxy,
                    model->getProxyIp(), model->getProxyPort());
        this->socket->setProxy(proxy);
    }
}
void Smtp::setAppTitle(const QString &title){
    appTitle = title;
}
void Smtp::setCC(const QString &v){
    cc = v;
}

void Smtp::sendMail(const QString &from, const QString &to, const QString &subject, const QString &body, QFile &file)
{
    emit status( tr( "Composing message..." ) );
    message = "MIME-Version: 1.0\n";
    message.append("Subject: " + subject + "\n");
    message.append("From: " + from + "\n");
    message.append("To: " + to + "\n");
    if(!cc.isEmpty())
        message.append("Cc: " + cc + "\n");


    //Let's intitiate multipart MIME with cutting boundary "frontier"
    message.append("Content-Type: multipart/mixed; boundary=frontier\n\n");



    message.append( "--frontier\n" );
    message.append( "Content-Type: text/html; charset=UTF-8\n\n" );  //Uncomment this for HTML formating, coment the line below
    //message.append( "Content-Type: text/plain\n\n" )
    //message.append( "Content-Type: text/html\n\n" );  //Uncomment this for HTML formating, coment the line below
    message.append(body);
    message.append("\n\n");

            if(file.exists())
            {
                if (!file.open(QIODevice::ReadOnly))
                {
                    qDebug("Couldn't open the file");
                    QMessageBox::warning( 0, appTitle , tr( "Couldn't open the file\n\n" )  );
                        return ;
                }
                QByteArray bytes = file.readAll();
                message.append( "--frontier\n" );
                message.append( "Content-Type: application/octet-stream\nContent-Disposition: attachment; filename="+ file.fileName() +";\nContent-Transfer-Encoding: base64\n\n" );
                message.append(bytes.toBase64());
                message.append("\n");
            }else{
            }


    message.append( "--frontier--\n" );

    message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1( "\r\n" ) );
    message.replace( QString::fromLatin1( "\r\n.\r\n" ),QString::fromLatin1( "\r\n..\r\n" ) );


    this->from = from;
    rcpt = to;
    state = Init;
    connectToHost();

    /*
     * Gmail not trusted App - key loading
    QSslConfiguration sslCfg = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> ca_list = sslCfg.caCertificates();
    QList<QSslCertificate> ca_new = QSslCertificate::fromData("CaCertificates");
    ca_list += ca_new;
    sslCfg.setCaCertificates(ca_list);
    sslCfg.setProtocol( QSsl::SslV3 );
    QSslConfiguration::setDefaultConfiguration(sslCfg);
    */



}
void Smtp::connectToHost()
{
    emit status( tr( "Connecting to host..." ) );
    socket->connectToHostEncrypted(host, port); //"smtp.gmail.com" and 465 for gmail TLS
    if (!socket->waitForConnected(timeout)) {
    //     qDebug() << socket->errorString();
     }
    t = new QTextStream( socket );
}

Smtp::~Smtp()
{
    delete t;
    delete socket;
}
void Smtp::stateChanged(QAbstractSocket::SocketState socketState)
{
    switch(socketState){
    case (QAbstractSocket::UnconnectedState):
        //emit status( tr( "Host unconnected" ) );
      break;
    case (QAbstractSocket::HostLookupState):
        emit status( tr( "Looking up host..." ) );
      break;
    case (QAbstractSocket::ConnectingState):
        emit status( tr( "Connecting to host..." ) );
      break;
    case (QAbstractSocket::ConnectedState):
        emit status( tr( "Host connected..." ) );
      break;
    case (QAbstractSocket::BoundState):
      break;
    case (QAbstractSocket::ListeningState):
      break;
    case (QAbstractSocket::ClosingState):
        //emit status( tr( "Closing connection..." ) );
      break;
    }
    //qDebug() <<"stateChanged " << socketState;
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError)
{
   // qDebug() << "error " << socketError;
}

void Smtp::disconnected()
{

   // qDebug() <<"disconneted";
   // qDebug() << "error "  << socket->errorString();
}

void Smtp::connected()
{
    emit status( tr( "Connected." ) );
   // qDebug() << "Connected ";
}
void Smtp::stop()
{
    if(socket->isOpen())
        socket->abort();
}

void Smtp::readyRead()
{


   // qDebug() <<"readyRead";
    // SMTP is line-oriented

    QString responseLine;
    do
    {
        responseLine = socket->readLine();
        response += responseLine;
    }
    while ( socket->canReadLine() && responseLine[3] != ' ' );

    responseLine.truncate( 3 );

  //  qDebug() << "Server response code:" <<  responseLine;
  //  qDebug() << "Server response: " << response;

    if ( state == Init && responseLine == "220" )
    {
        // banner was okay, let's go on
        *t << "EHLO localhost" <<"\r\n"; t->flush();
        emit status( tr( "Connected to server" ) );
        state = HandShake;
    }
    //No need, because we using socket->startClienEncryption() which makes the SSL handshake for you
    /*
    else if (state == Tls && responseLine == "250")
    {
        // Trying AUTH
        qDebug() << "STarting Tls";
        *t << "STARTTLS" << "\r\n";  t->flush();
        state = HandShake;
    }
    //*/
    else if (state == HandShake && responseLine == "250")
    {
        socket->startClientEncryption();
        if(!socket->waitForEncrypted(timeout))
        {
  //          qDebug() << socket->errorString();
            state = Close;
        }
        emit status( tr( "Connected to server" ) );
        //Send EHLO once again but now encrypted

        *t << "EHLO localhost" << "\r\n"; t->flush();
        state = Auth;
    }
    else if (state == Auth && responseLine == "250")
    {
        // Trying AUTH
   //     qDebug() << "Auth";
        *t << "AUTH LOGIN" << "\r\n"; t->flush();
        emit status( tr( "Authenticating..." ) );
        state = User;
    }
    else if (state == User && responseLine == "334")
    {
        //Trying User        
  //      qDebug() << "Username";
        //GMAIL is using XOAUTH2 protocol, which basically means that password and username has to be sent in base64 coding
        //https://developers.google.com/gmail/xoauth2_protocol
        *t << QByteArray().append(user).toBase64()  << "\r\n"; t->flush();
        emit status( tr( "Authenticating..." ) );
        state = Pass;
    }
    else if (state == Pass && responseLine == "334")
    {
  //      qDebug() << "Pass";
        *t << QByteArray().append(pass).toBase64() << "\r\n"; t->flush(); //Trying pass
        emit status( tr( "User authenticated." ) );
        state = Mail;
    }
    else if ( state == Mail && responseLine == "235" )
    {
        // HELO response was okay (well, it has to be)
        //Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated the following way -> <email@gmail.com>
   //     qDebug() << "MAIL FROM:<" << from << ">";
        *t << "MAIL FROM:<" << from << ">\r\n"; t->flush();
        emit status( tr( "Composing send request..." ) );
        state = Rcpt;
    }
    else if ( state == Rcpt && responseLine == "250" )
    {
        //Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated the following way -> <email@gmail.com>
        *t << "RCPT TO:<" << rcpt << ">\r\n";   t->flush();
        emit status( tr( "Composing send request..." ) );
        state = Data;
    }
    else if ( state == Data && responseLine == "250" )
    {

        *t << "DATA\r\n"; t->flush();
        emit status( tr( "Composing send request..." ) );
        state = Body;
    }
    else if ( state == Body && responseLine == "354" )
    {

        *t << message << "\r\n.\r\n"; t->flush();
        emit status( tr( "Message attached." ) );
        state = Quit;
    }
    else if ( state == Quit && responseLine == "250" )
    {

        if(socket->isOpen()){
            *t << "QUIT\r\n"; t->flush(); // close connection.
            state = Close;
            emit status( tr( "Message sent." ) );
            emit finished( tr( "Message sent." ) );
        }
    }
    else if ( state == Close )
    {
        deleteLater();
        return;
    }
    else
    {
        if(responseLine == "534" && (this->host == "smtp.gmail.com" || this->host == "smtp-relay.gmail.com")){
            QMessageBox::warning( 0,appTitle, tr( "Google require to allow access for third-part application.\nPlease log into your google email account\nand go to: Account Settings->Signing in->\"Access for less secure apps\"\nChange settings of \"Less secure apps\" from\"Disabled\" to \"ON\"\n\nOr follow this link: \nhttps://www.google.com/settings/security/lesssecureapps and set \"Access for less secure apps\" to ON. " ));
            emit error( tr( "Failed to login" ) );
        }
        else{
            // something broke.
            QMessageBox::warning( 0,appTitle, tr( "Unexpected reply from SMTP server:\n\n" ) + response );
            emit error( tr( "Failed to send message" ) );
        }
        if(socket->isOpen())
            *t << "QUIT\r\n"; t->flush();
        state = Close;
    }
    response = "";
}
