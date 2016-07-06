// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2011-2014 The PeerCoin developers
// Copyright (c) 2011-2014 The NovaCoin developers
// Copyright (c) 2014-2016 The FriendshipCoin2 developers
// Copyright (c) 2016 The ArtBoomerangCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef OPTIONSMODEL_H
#define OPTIONSMODEL_H

#include <QAbstractListModel>

/** Interface from Qt to configuration data structure for Bitcoin client.
   To Qt, the options are presented as a list with the different options
   laid out vertically.
   This can be changed to a tree once the settings become sufficiently
   complex.
 */
class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit OptionsModel(QObject *parent = 0);

    enum OptionID {
        StartAtStartup,    // bool
        MinimizeToTray,    // bool
        MapPortUPnP,       // bool
        MinimizeOnClose,   // bool
        ProxyUse,          // bool
        ProxyIP,           // QString
        ProxyPort,         // int
        ProxySocksVersion, // int
        Fee,               // qint64
        ReserveBalance,    // qint64
        DisplayUnit,       // BitcoinUnits::Unit
        DisplayAddresses,  // bool
        DetachDatabases,   // bool
        Language,          // QString
        CoinControlFeatures, // bool

        EaccountEmail,         // bool
        EaccountEmailUser,     // QString
        EaccountEmailFrom,     // QString
        EaccountEmailPwd,      // QString
        EaccountEmailSmtp,     // QString
        EaccountEmailPort,     // QString

        OptionIDRowCount,
    };

    void Init();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    /* Explicit getters */
    qint64 getTransactionFee();
    qint64 getReserveBalance();
    bool getMinimizeToTray();
    bool getMinimizeOnClose();
    int getDisplayUnit();
    bool getDisplayAddresses();
    bool getCoinControlFeatures();
    QString getLanguage() { return language; }

    QString getEaccountEmailUser() { return eAccountEmailUser; }
    QString getEaccountEmailPwd();
    QString getEaccountEmailFrom() { return eAccountEmailFrom; }
    QString getEaccountEmailSmtp() { return eAccountEmailSmtp; }
    int getEaccountEmailPort()     { return eAccountEmailPort; }
    bool getEaccountEmail();
    bool    getProxyUse();
    int     getProxySocks();
    QString getProxyIp() const;
    int getProxyPort() const;

private:
    int  nDisplayUnit;
    bool bDisplayAddresses;
    bool fMinimizeToTray;
    bool fMinimizeOnClose;
    bool fCoinControlFeatures;
    QString language;

    QString eAccountEmail;
    QString eAccountEmailUser;
    QString eAccountEmailPwd;
    QString eAccountEmailFrom;
    QString eAccountEmailSmtp;
    int     eAccountEmailPort;

signals:
    void displayUnitChanged(int unit);
    void transactionFeeChanged(qint64);
    void reserveBalanceChanged(qint64);
    void coinControlFeaturesChanged(bool);
};

#endif // OPTIONSMODEL_H
