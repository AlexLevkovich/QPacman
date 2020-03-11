/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "alpmoptionswidget.h"
#include "ui_alpmoptionswidget.h"
#include "static.h"
#include <QNetworkProxy>
#include "alpmconfig.h"

AlpmOptionsWidget::AlpmOptionsWidget(QWidget *parent) : CategoryWidget(parent), ui(new Ui::AlpmOptionsWidget) {
    ui->setupUi(this);

    ui->proxyTypeCombo->setItemData(0,QNetworkProxy::NoProxy);
    ui->proxyTypeCombo->setItemData(1,QNetworkProxy::HttpCachingProxy);
    ui->proxyTypeCombo->setItemData(2,QNetworkProxy::HttpProxy);
    ui->proxyTypeCombo->setItemData(3,QNetworkProxy::FtpCachingProxy);
    QNetworkProxy proxy = AlpmConfig::downloaderProxy();
    for (int i=0;i<ui->proxyTypeCombo->count();i++) {
        if (ui->proxyTypeCombo->itemData(i).toInt() == proxy.type()) {
            ui->proxyTypeCombo->setCurrentIndex(i);
            on_proxyTypeCombo_activated(i);
            break;
        }
    }

    ui->proxyAddressLine->setText(proxy.hostName());
    ui->proxyPortSpin->setValue(proxy.port());
    ui->proxyUserLine->setText(proxy.user());
    ui->proxyPasswordLine->setText(proxy.password());
    ui->threadsSpin->setValue(AlpmConfig::downloaderThreadCount());
    ui->timeoutSpin->setValue(AlpmConfig::downloaderTimeout()/1000);
}

AlpmOptionsWidget::~AlpmOptionsWidget() {
    delete ui;
}

void AlpmOptionsWidget::okPressed() {
    QNetworkProxy proxy;
    proxy.setType((QNetworkProxy::ProxyType)ui->proxyTypeCombo->itemData(ui->proxyTypeCombo->currentIndex()).toInt());
    proxy.setHostName(ui->proxyAddressLine->text());
    proxy.setPort(ui->proxyPortSpin->value());
    proxy.setUser(ui->proxyUserLine->text());
    proxy.setPassword(ui->proxyPasswordLine->text());
    AlpmConfig::setDownloaderProxy(proxy);
    AlpmConfig::setDownloaderTimeout(ui->timeoutSpin->value()*1000);
    AlpmConfig::setDownloaderThreads(ui->threadsSpin->value());
}

void AlpmOptionsWidget::on_proxyTypeCombo_activated(int index) {
    bool disable = (ui->proxyTypeCombo->itemData(index).toInt() == QNetworkProxy::NoProxy);
    ui->proxyAddressLine->setDisabled(disable);
    ui->proxyPasswordLine->setDisabled(disable);
    ui->proxyPortSpin->setDisabled(disable);
    ui->proxyUserLine->setDisabled(disable);
}
