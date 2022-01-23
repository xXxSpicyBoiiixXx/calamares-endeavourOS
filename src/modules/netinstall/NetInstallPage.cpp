/*
 *   SPDX-FileCopyrightText: 2016 Luca Giambonini <almack@chakraos.org>
 *   SPDX-FileCopyrightText: 2016 Lisa Vitolo     <shainer@chakraos.org>
 *   SPDX-FileCopyrightText: 2017 Kyle Robbertze  <krobbertze@gmail.com>
 *   SPDX-FileCopyrightText: 2017-2018 2020, Adriaan de Groot <groot@kde.org>
 *   SPDX-FileCopyrightText: 2017 Gabriel Craciunescu <crazy@frugalware.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "NetInstallPage.h"

#include "PackageModel.h"
#include "ui_page_netinst.h"

#include "GlobalStorage.h"
#include "JobQueue.h"

#include "network/Manager.h"
#include "utils/Logger.h"
#include "utils/Retranslator.h"
#include "utils/Yaml.h"

#include <QHeaderView>
#include <QNetworkReply>

NetInstallPage::NetInstallPage( Config* c, QWidget* parent )
    : QWidget( parent )
    , m_config( c )
    , ui( new Ui::Page_NetInst )
{
    ui->setupUi( this );
    ui->groupswidget->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
    ui->groupswidget->setModel( c->model() );
    connect( c, &Config::statusChanged, ui->netinst_status, &QLabel::setText );
    connect( c, &Config::titleLabelChanged, [ui = this->ui]( const QString title ) {
        ui->label->setVisible( !title.isEmpty() );
        ui->label->setText( title );
    } );
    connect( c, &Config::statusReady, this, &NetInstallPage::expandGroups );
}

NetInstallPage::~NetInstallPage() {}

void
NetInstallPage::expandGroups()
{
    auto* model = m_config->model();
    // Go backwards because expanding a group may cause rows to appear below it
    for ( int i = model->rowCount() - 1; i >= 0; --i )
    {
        auto index = model->index( i, 0 );
        if ( model->data( index, PackageModel::MetaExpandRole ).toBool() )
        {
            ui->groupswidget->setExpanded( index, true );
        }
    }
}

void
NetInstallPage::onActivate()
{
    ui->groupswidget->setFocus();

    // The NetInstallSelect global sotrage value can be used to make additional items selected by default
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    if ( gs->contains( "NetinstallSelect" ) && gs->value( "NetinstallSelect" ).canConvert( QVariant::StringList ) )
    {
        const QStringList selectNames = gs->value( "NetinstallSelect" ).toStringList();

        static_cast< PackageModel* >( ui->groupswidget->model() )->setSelections( selectNames );
    }

    // If NetInstallAdd is found in global storage, add those items to the tree
    if ( gs->contains( "NetinstallAdd" ) && gs->value( "NetinstallAdd" ).canConvert( QVariant::List ) )
    {
        const QVariantList groups = gs->value( "NetinstallAdd" ).toList();

        static_cast< PackageModel* >( ui->groupswidget->model() )->appendModelData( groups, "packageChooser" );
    }
}
