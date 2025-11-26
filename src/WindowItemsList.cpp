/*
 * Copyright (C) 2022-2025 Anonymous Idiot
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <QAction>
#include <QCloseEvent>
#include <QHeaderView>
#include <QMenu>
#include <QSettings>

#include "WindowItemsList.h"

#include "SLFFile.h"
#include "STI.h"
#include "main.h"

#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>

#include "Screen.h"
#include "DialogChooseColumns.h"

#include "WButton.h"
#include "WCheckBox.h"
#include "WDDL.h"
#include "WImage.h"
#include "WItem.h"
#include "WLabel.h"
#include "WScrollBar.h"
#include "WTableWidget.h"
#include "WTableWidgetItem.h"

#include "spell.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    CB_FILTER_BY_PROF,
    CB_FILTER_BY_RACE,
    CB_FILTER_BY_SEX,

    DDL_PROFS,
    DDL_RACES,
    DDL_GENDERS,

    TABLE_ITEMS,

    SIZE_WIDGET_IDS
} widget_ids;


WindowItemsList::WindowItemsList(character::profession profession, character::race race, character::gender gender)
    : QWidget(),
    Wizardry8Scalable(::getAppScale()),
    m_prof_filter(profession),
    m_race_filter(race),
    m_gender_filter(gender),
    m_contextMenu(NULL)
{
    QPixmap controlsBg = makeDialogForm();

    // Tile the background image that was loaded as part of making the controlBg
    QBrush   b( m_bgImg.toImage() );
    QPalette pal = QPalette( this->palette() );

    pal.setBrush( QPalette::Disabled, QPalette::Window, b );
    pal.setBrush( QPalette::Active,   QPalette::Window, b );
    pal.setBrush( QPalette::Inactive, QPalette::Window, b );
    pal.setBrush( QPalette::Normal,   QPalette::Window, b );

    setBackgroundRole( QPalette::Window );
    setPalette( pal );

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout listItemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    controlsBg,                                                        this ),  -1,  NULL },

        { TABLE_ITEMS,        QRect(  10, 145, 400, 200 ),    new WTableWidget( this ), -1, NULL },

        { CB_FILTER_BY_PROF,  QRect(  20,  19, 140,  13 ),    new WCheckBox( StringList::FilterByProfession + StringList::APPEND_COLON,         this ),  -1,  SLOT(filterProf(int)) },
        { CB_FILTER_BY_RACE,  QRect(  20,  60, 140,  13 ),    new WCheckBox( StringList::FilterByRace + StringList::APPEND_COLON,               this ),  -1,  SLOT(filterRace(int)) },
        { CB_FILTER_BY_SEX,   QRect(  20, 101, 140,  13 ),    new WCheckBox( StringList::FilterByGender + StringList::APPEND_COLON,             this ),  -1,  SLOT(filterSex(int)) },

        // Do these in reverse order because each drop down list obscures the one below it, and they need to
        // be on top to do it.
        { DDL_GENDERS,        QRect( 169,  94,  -1,  -1 ),    new WDDL(      "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,       this ),  -1,  SLOT(ddlChanged(int)) },
        { DDL_RACES,          QRect( 169,  53,  -1,  -1 ),    new WDDL(      "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,       this ),  -1,  SLOT(ddlChanged(int)) },
        { DDL_PROFS,          QRect( 169,  12,  -1,  -1 ),    new WDDL(      "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,       this ),  -1,  SLOT(ddlChanged(int)) },
    };

    int num_widgets = sizeof(listItemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( listItemsScrn, num_widgets, this );

    if (WCheckBox *q = qobject_cast<WCheckBox *>(m_widgets[ CB_FILTER_BY_PROF ]))
    {
        q->setCheckState( (m_prof_filter == (character::profession) 0) ? Qt::Unchecked : Qt::Checked );
    }
    if (WCheckBox *q = qobject_cast<WCheckBox *>(m_widgets[ CB_FILTER_BY_RACE ]))
    {
        q->setCheckState( (m_race_filter == (character::race) 0) ? Qt::Unchecked : Qt::Checked );
    }
    if (WCheckBox *q = qobject_cast<WCheckBox *>(m_widgets[ CB_FILTER_BY_SEX ]))
    {
        q->setCheckState( (m_gender_filter == (character::gender) 0) ? Qt::Unchecked : Qt::Checked );
    }

    if (WDDL *q = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ]))
    {
        populateDDLProfessions( q );

        connect( q, SIGNAL(listActive()), this, SLOT(ddlActive()) );
        if (m_prof_filter != (character::profession) 0)
        {
            QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

            for (int k=0; k<metaProf.keyCount(); k++)
            {
                if (m_prof_filter == static_cast<character::profession>( metaProf.value(k) ))
                {
                    q->setCurrentRow( k );
                }
            }
        }
        q->updateList();
    }
    if (WDDL *q = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ]))
    {
        populateDDLRaces( q );

        connect( q, SIGNAL(listActive()), this, SLOT(ddlActive()) );
        if (m_race_filter != (character::race) 0)
        {
            QMetaEnum metaRace = QMetaEnum::fromType<character::race>();

            for (int k=0; k<metaRace.keyCount(); k++)
            {
                if (m_race_filter == static_cast<character::race>( metaRace.value(k) ))
                {
                    q->setCurrentRow( k );
                }
            }
        }
        q->updateList();
    }
    if (WDDL *q = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ]))
    {
        populateDDLGenders( q );

        connect( q, SIGNAL(listActive()), this, SLOT(ddlActive()) );
        if (m_gender_filter != (character::gender) 0)
        {
            QMetaEnum metaGender = QMetaEnum::fromType<character::gender>();

            for (int k=0; k<metaGender.keyCount(); k++)
            {
                if (m_gender_filter == static_cast<character::gender>( metaGender.value(k) ))
                {
                    q->setCurrentRow( k );
                }
            }
        }
        q->updateList();
    }

    if (QTableWidget *q = qobject_cast<QTableWidget *>(m_widgets[ TABLE_ITEMS ]))
    {
        Q_ASSERT( q->horizontalHeader() );
        q->horizontalHeader()->setSectionsMovable( true );
        q->setSelectionBehavior( QAbstractItemView::SelectRows );

        q->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( q->horizontalHeader(), SIGNAL(customContextMenuRequested(QPoint)),
                 this, SLOT(tableMenu(QPoint)));

        q->verticalHeader()->setVisible( false );

        q->setSelectionMode( QAbstractItemView::SingleSelection );
        q->setDragEnabled( true );

        populateColumns();
    }

    updateFilter();

    this->setMinimumSize( 420 * m_scale, 355 * m_scale );

    show();
}

WindowItemsList::~WindowItemsList()
{
    // context menu is destroyed when window is, and the action
    // is destroyed when the menu is.
}

QList<DialogChooseColumns::column> WindowItemsList::loadColumnsFromRegistry()
{
    QList<DialogChooseColumns::column> cols;

    QSettings settings;

    QVariant v = settings.value("Items List Columns");

    if (v.isNull())
    {
        // Defaults
        cols << DialogChooseColumns::Name
             << DialogChooseColumns::Type
             << DialogChooseColumns::AC
             << DialogChooseColumns::Special;
    }
    else
    {
        QString s = v.toString();
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QStringList sl = s.split(',');
#else
        QStringList sl = s.split(',', Qt::SkipEmptyParts);
#endif

        for (int k=0; k < sl.size(); k++)
        {
            cols << static_cast<DialogChooseColumns::column>( sl[k].toShort() );
        }
    }

    return cols;
}

void WindowItemsList::populateColumns()
{
    if (QTableWidget *q = qobject_cast<QTableWidget *>(m_widgets[ TABLE_ITEMS ]))
    {
        QList<DialogChooseColumns::column> cols = loadColumnsFromRegistry();

        QStringList headers;
        for (int k=0; k < cols.size(); k++)
        {
            headers << ::getBaseStringTable()->getString( cols[k] );
        }

        q->setColumnCount( cols.size() );
        q->setHorizontalHeaderLabels( headers );
    }
}

void WindowItemsList::tableMenu(QPoint pos)
{
    if (m_contextMenu == NULL)
    {
        m_contextMenu = new QMenu(this);

        QAction *actionChooseCols = m_contextMenu->addAction( "Choose Columns" );
        if (actionChooseCols)
        {
            connect( actionChooseCols, &QAction::triggered, this, &WindowItemsList::chooseColumns );
        }
    }

    Q_ASSERT( qobject_cast<QHeaderView *>( sender() ) );
    m_contextMenu->popup( qobject_cast<QHeaderView *>( sender() )->viewport()->mapToGlobal( pos ) );
}

void WindowItemsList::closeEvent(QCloseEvent *event)
{
    emit windowClosing();
    Q_ASSERT( event );
    event->accept();
}

void WindowItemsList::chooseColumns()
{
    QList<DialogChooseColumns::column> cols = loadColumnsFromRegistry();

    DialogChooseColumns d( cols, this );

    if (d.exec() == QDialog::Accepted)
    {
        cols = d.getColumns();

        if (cols.isEmpty())
            cols << DialogChooseColumns::Name;

        QString s;

        for (int k=0; k < cols.size(); k++)
        {
            s += "," + QString::number( cols[k] );
        }
        QSettings settings;

        Q_ASSERT( s.length() > 1 );
        settings.setValue( "Items List Columns", s.mid(1) );

        populateColumns();
        updateFilter();
    }
}

void WindowItemsList::setScale(double scale)
{
    m_scale = scale;

    QList<QWidget *> widgets = this->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    for (int k=0; k<widgets.size(); k++)
    {
        // No, we can't go straight to this in the findChildren() call above, because
        // it's a slim 'interface' that doesn't inherit from QObject
        if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(widgets[k]))
        {
            w->setScale( m_scale );
        }
    }

    this->resize( sizeHint() );

    this->update();
}

void WindowItemsList::resizeEvent(QResizeEvent *event)
{
    Q_ASSERT(event);

    int new_width  = event->size().width();
    int new_height = event->size().height();

    // The table view consumes ALL the extra space in the resized window, rather
    // than rescaling the window itself like we do for all other windows in this
    // app. this is because it is more useful here to be able to make more space
    // for new columns to be added, or columns to be made wider instead.
    if (QTableWidget *q = qobject_cast<QTableWidget *>(m_widgets[ TABLE_ITEMS ]))
    {
        // The table initially starts off at position 10 * m_scale, 145 * m_scale
        // and has initial dimensions 400 * m_scale x 200 * m_scale.
        // It will continue to stay at the same position but we change its size:

        q->resize( new_width - (10 + 10) * m_scale,
                   new_height - (145 + 10) * m_scale );
    }
}

QPixmap WindowItemsList::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap  bgDdl;

    // images used in dialog
    m_bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );
    bgDdl   = SLFFile::getPixmapFromSlf( "CHAR GENERATION/CG_PROFESSION.STI", 0 );

    QPixmap customImage( QSize( 420, 145 ) );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, m_bgImg,                      0,                       0, m_bgImg.width(), 145 );
    p.drawPixmap( 240,   0, m_bgImg,  m_bgImg.width() - 180,                       0,             180, 145 );
    p.drawPixmap(   0, 125, m_bgImg,                      0,   m_bgImg.height() - 20, m_bgImg.width(),  20 );
    p.drawPixmap( 240, 125, m_bgImg,  m_bgImg.width() - 180,   m_bgImg.height() - 20,             180,  20 );

    p.drawPixmap( 164,   4, bgDdl,  15,   4, 200,  46 );
    p.drawPixmap( 164,  45, bgDdl,  15,  55, 200,  46 );
    p.drawPixmap( 164,  86, bgDdl,  15, 106, 200,  40 );

    p.end();

    return customImage;
}

void WindowItemsList::setVisible( bool visible )
{
    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(kids.at(k)))
        {
            q->setVisible( visible );
        }
    }

    QWidget::setVisible( visible );
}

void WindowItemsList::populateDDLProfessions(WDDL *ddl)
{
    if (ddl)
    {
        QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

        SLFFile ic( "CHAR GENERATION/CG_ICONS_PROFESSION.STI" );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array;

            ic.readAll( array );
            STI c( array );

            for (int k=0; k<metaProf.keyCount(); k++)
            {
                character::profession p = static_cast<character::profession>( metaProf.value(k) );

                int  imIdx;

                switch (p)
                {
                    default:
                    case character::profession::Fighter:   imIdx = 10; break;
                    case character::profession::Lord:      imIdx = 24; break;
                    case character::profession::Valkyrie:  imIdx =  8; break;
                    case character::profession::Ranger:    imIdx =  2; break;
                    case character::profession::Samurai:   imIdx = 16; break;
                    case character::profession::Ninja:     imIdx = 20; break;
                    case character::profession::Monk:      imIdx = 12; break;
                    case character::profession::Rogue:     imIdx = 28; break;
                    case character::profession::Gadgeteer: imIdx =  6; break;
                    case character::profession::Bard:      imIdx =  0; break;
                    case character::profession::Priest:    imIdx = 18; break;
                    case character::profession::Alchemist: imIdx = 22; break;
                    case character::profession::Bishop:    imIdx =  4; break;
                    case character::profession::Psionic:   imIdx = 14; break;
                    case character::profession::Mage:      imIdx = 26; break;
                }

                QListWidgetItem *prof = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTProfessions + k ));
                prof->setData( Qt::DecorationRole, QPixmap::fromImage( c.getImage( imIdx ) ) );
                prof->setData( Qt::UserRole, p );

                ddl->addItem( prof );
            }

            ic.close();
        }
    }
}

void WindowItemsList::populateDDLRaces(WDDL *ddl)
{
    if (ddl)
    {
        QMetaEnum metaRace = QMetaEnum::fromType<character::race>();

        SLFFile ic( "CHAR GENERATION/CG_ICONS_RACE.STI" );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array;

            ic.readAll( array );
            STI c( array );

            for (int k=0; k<metaRace.keyCount(); k++)
            {
                character::race r = static_cast<character::race>( metaRace.value(k) );

                int  imIdx;

                switch (r)
                {
                    default:
                    case character::race::Human:     imIdx =  8; break;
                    case character::race::Elf:       imIdx =  4; break;
                    case character::race::Dwarf:     imIdx = 12; break;
                    case character::race::Gnome:     imIdx = 16; break;
                    case character::race::Hobbit:    imIdx = 14; break;
                    case character::race::Faerie:    imIdx = 20; break;
                    case character::race::Lizardman: imIdx =  0; break;
                    case character::race::Dracon:    imIdx = 10; break;
                    case character::race::Felpurr:   imIdx =  2; break;
                    case character::race::Rawulf:    imIdx = 18; break;
                    case character::race::Mook:      imIdx =  6; break;
                    case character::race::Trynnie:   imIdx = 30; break;
                    case character::race::TRang:     imIdx = 24; break;
                    case character::race::Umpani:    imIdx = 22; break;
                    case character::race::Rapax:     imIdx = 26; break;
                    case character::race::Android:   imIdx = 28; break;
                }

                QListWidgetItem *race = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTRaces + k ));
                race->setData( Qt::DecorationRole, QPixmap::fromImage( c.getImage( imIdx ) ) );
                race->setData( Qt::UserRole, r );

                ddl->addItem( race );
            }

            ic.close();
        }
    }
}

void WindowItemsList::populateDDLGenders(WDDL *ddl)
{
    if (ddl)
    {
        QMetaEnum metaGender = QMetaEnum::fromType<character::gender>();

        SLFFile ic( "CHAR GENERATION/CG_ICONS_GENDER.STI" );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array;

            ic.readAll( array );
            STI c( array );

            for (int k=0; k<metaGender.keyCount(); k++)
            {
                character::gender g = static_cast<character::gender>( metaGender.value(k) );

                int  imIdx;

                switch (g)
                {
                    default:
                    case character::gender::Male:     imIdx =  0; break;
                    case character::gender::Female:   imIdx =  2; break;
                }

                QListWidgetItem *gender = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTGenders + k ));
                gender->setData( Qt::DecorationRole, QPixmap::fromImage( c.getImage( imIdx ) ) );
                gender->setData( Qt::UserRole, g );

                ddl->addItem( gender );
            }

            ic.close();
        }
    }
}

void WindowItemsList::ddlChanged(int value)
{
    if (sender() == m_widgets[ DDL_PROFS ])
        m_prof_filter = static_cast<character::profession> (value);
    else if (sender() == m_widgets[ DDL_RACES ])
        m_race_filter = static_cast<character::race> (value);
    else if (sender() == m_widgets[ DDL_GENDERS ])
        m_gender_filter = static_cast<character::gender> (value);

    updateFilter();
}

void WindowItemsList::ddlActive()
{
    if (sender() != m_widgets[ DDL_PROFS ])
    {
        if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ] ))
        {
            ddl->showDDL( false );
        }
    }
    if (sender() != m_widgets[ DDL_RACES ])
    {
        if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ] ))
        {
            ddl->showDDL( false );
        }
    }
    if (sender() != m_widgets[ DDL_GENDERS ])
    {
        if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ] ))
        {
            ddl->showDDL( false );
        }
    }
}

void WindowItemsList::filterProf(int state)
{
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ] ))
    {
        ddl->setEnabled( state == Qt::Checked );
        if (state == Qt::Checked)
        {
            if (m_prof_filter == (character::profession) 0)
            {
                // reset the profession filter from whatever the list is showing
                m_prof_filter = static_cast<character::profession> (ddl->getValue());
            }
        }
        else
        {
            m_prof_filter = (character::profession) 0;
        }
        updateFilter();
    }
}

void WindowItemsList::filterRace(int state)
{
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ] ))
    {
        ddl->setEnabled( state == Qt::Checked );
        if (state == Qt::Checked)
        {
            if (m_race_filter == (character::race) 0)
            {
                // reset the race filter from whatever the list is showing
                m_race_filter = static_cast<character::race> (ddl->getValue());
            }
        }
        else
        {
            m_race_filter = (character::race) 0;
        }
        updateFilter();
    }
}

void WindowItemsList::filterSex(int state)
{
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ] ))
    {
        ddl->setEnabled( state == Qt::Checked );
        if (state == Qt::Checked)
        {
            if (m_gender_filter == (character::gender) 0)
            {
                // reset the gender filter from whatever the list is showing
                m_gender_filter = static_cast<character::gender> (ddl->getValue());
            }
        }
        else
        {
            m_gender_filter = (character::gender) 0;
        }
        updateFilter();
    }
}

void WindowItemsList::updateFilter()
{
    QList<quint16> items = dbHelper::getHelper()->getFilteredItems( m_prof_filter, m_race_filter, m_gender_filter );

    if (QTableWidget *q = qobject_cast<QTableWidget *>(m_widgets[ TABLE_ITEMS ]))
    {
        q->clearContents();
        q->setRowCount( items.size() );

        q->setSortingEnabled( false );

        QFont wizFont("Wizardry", 9 * m_scale, QFont::Thin);

        for (int i=0; i < items.size(); i++)
        {
            item    rowitem(items[i]);

            QList<DialogChooseColumns::column> cols = loadColumnsFromRegistry();

            for (int k=0; k < cols.size(); k++)
            {
                bool    numeric = false;
                QString prop = lookupItemProperty( &rowitem, cols[k], &numeric );
                WTableWidgetItem *cell = new WTableWidgetItem( prop );

                Qt::ItemFlags flags = cell->flags();

                flags &= ~Qt::ItemIsEditable; // all cells are read only

                if (cols[k] == DialogChooseColumns::Name)
                {
                    flags |= Qt::ItemIsDragEnabled;
                    cell->setData( Qt::UserRole, items[i] );
                }
                else
                    flags &= ~Qt::ItemIsDragEnabled;

                cell->setFlags( flags );

                cell->setFont(wizFont);
                cell->setTextAlignment( (numeric ? Qt::AlignRight : Qt::AlignLeft ) | Qt::AlignVCenter);

                q->setItem( i, k, cell );
            }
        }
        q->setSortingEnabled( true );
    }
}

QString WindowItemsList::lookupItemProperty( item *i, DialogChooseColumns::column col, bool *numeric)
{
    QString prop = "";

    if (numeric)
        *numeric = false;

    switch (col)
    {
        case DialogChooseColumns::Name:
        {
            prop = i->getName();
            break;
        }

        case DialogChooseColumns::Type:
        {
            prop = ::getBaseStringTable()->getString( StringList::LISTItemTypes + i->getType() );
            break;
        }

        case DialogChooseColumns::Equippable:
        {
            switch (i->getType())
            {
                case item::type::ShortWeapon:
                case item::type::ExtendedWeapon:
                case item::type::ThrownWeapon:
                case item::type::RangedWeapon:
                    prop = ::getBaseStringTable()->getString( StringList::PrimaryWeapon );
                    if (i->canSecondary())
                    {
                        prop += ", " + ::getBaseStringTable()->getString( StringList::SecondaryWeapon );
                    }
                    break;

                case item::type::Ammunition: // should have secondary set - but don't
                case item::type::Shield: // should have secondary set - but don't
                    prop = ::getBaseStringTable()->getString( StringList::SecondaryWeapon );
                    break;

                case item::type::TorsoArmor:
                    prop = ::getBaseStringTable()->getString( StringList::Torso );
                    break;

                case item::type::LegArmor:
                    prop = ::getBaseStringTable()->getString( StringList::Legs );
                    break;

                case item::type::HeadGear:
                    prop = ::getBaseStringTable()->getString( StringList::Head );
                    break;

                case item::type::Gloves:
                    prop = ::getBaseStringTable()->getString( StringList::Hands );
                    break;

                case item::type::Shoes:
                    prop = ::getBaseStringTable()->getString( StringList::Feet );
                    break;

                case item::type::MiscEquipment:
                    prop = ::getBaseStringTable()->getString( StringList::MiscItem1 ) + ", " + ::getBaseStringTable()->getString( StringList::MiscItem2 );
                    break;

                case item::type::Cloak:
                    prop = ::getBaseStringTable()->getString( StringList::Cloak );
                    break;

                default:
                    break;
            }
            break;
        }

        case DialogChooseColumns::AC:
        {
            if (numeric)
                *numeric = true;

            if (i->getAC() > 0)
                prop = QString( "+%1" ).arg( i->getAC() );
            else if (i->getAC() < 0)
                prop = QString( "%1" ).arg( i->getAC() );
            break;
        }

        case DialogChooseColumns::Weight:
        {
            if (numeric)
                *numeric = true;

            prop = QString( tr("%1 pounds") ).arg( i->getWeight(), 0, 'f', 1 );
            break;
        }

        case DialogChooseColumns::WeightClass:
        {
            prop = i->getArmorWeightClassString();
            break;
        }

        case DialogChooseColumns::Damage:
        {
            quint16 min_damage, max_damage;
            int     percentage;

            if (numeric)
                *numeric = true;

            i->getDamage( &min_damage, &max_damage, &percentage );
            if (percentage != 0)
                prop = QString( "+%1%" ).arg(percentage);
            else if (max_damage != 0)
                prop = QString( "%1 - %2" ).arg(min_damage).arg(max_damage);
            break;
        }

        case DialogChooseColumns::ToHit:
        {
            if (numeric)
                *numeric = true;

            if (i->getToHit() > 0)
                prop = QString( "+%1" ).arg( i->getToHit() );
            else if (i->getToHit() < 0)
                prop = QString( "%1" ).arg( i->getToHit() );
            break;
        }

        case DialogChooseColumns::Initiative:
        {
            if (numeric)
                *numeric = true;

            if (i->getInitiative() > 0)
                prop = QString( "+%1" ).arg( i->getInitiative() );
            else if (i->getInitiative() < 0)
                prop = QString( "%1" ).arg( i->getInitiative() );
            break;
        }

        case DialogChooseColumns::Hands:
        {
            if (i->canSecondary())
                prop = "Primary or Secondary";
            else if (i->needs2Hands())
                prop = "2 Handed Weapon";
            else if ((i->getType() == item::type::ShortWeapon) ||
                     (i->getType() == item::type::ExtendedWeapon) ||
                     (i->getType() == item::type::ThrownWeapon) ||
                     (i->getType() == item::type::RangedWeapon))
            {
                prop = "Primary";
            }
            break;
        }

        case DialogChooseColumns::Cursed:
        {
            switch (i->getType())
            {
                case item::type::ShortWeapon:
                case item::type::ExtendedWeapon:
                case item::type::ThrownWeapon:
                case item::type::RangedWeapon:
                case item::type::Shield:
                case item::type::TorsoArmor:
                case item::type::LegArmor:
                case item::type::HeadGear:
                case item::type::Gloves:
                case item::type::Shoes:
                case item::type::MiscEquipment:
                case item::type::Cloak:
                    if (! i->isCursed() )
                        prop = "No";
                    else
                        prop = "Yes";
                    break;

                default:
                    break;
            }
            break;
        }

        case DialogChooseColumns::SpecialAttack:
        {
            prop = i->getSpecialAttackString();
            break;
        }

        case DialogChooseColumns::DoubleDamage:
        {
            prop = i->getSlaysString();
            break;
        }

        case DialogChooseColumns::Price:
        {
            if (numeric)
                *numeric = true;

             prop = QString( "%1" ).arg( i->getPrice() );
             break;
        }

        case DialogChooseColumns::Stackable:
        {
             prop = i->isStackable() ? "Yes" : "No";
             break;
        }

        case DialogChooseColumns::SkillsUsed:
        {
             prop = i->getSkillUsedString();
             break;
        }

        case DialogChooseColumns::BonusSwings:
        {
            if (numeric)
                *numeric = true;

            if (i->getBonusSwings() > 0)
                prop = QString( "+%1" ).arg( i->getBonusSwings() );
            else if (i->getBonusSwings() < 0)
                prop = QString( "%1" ).arg( i->getBonusSwings() );
            break;
        }

        case DialogChooseColumns::Spell:
        {
            int power;
            spell spl = i->getSpell(&power);
            QString spell_name = spl.getName();
            if (spell_name.size() > 0)
            {
                if (i->getType() == item::type::Spellbook)
                {
                    // Don't show a power level because spellbooks don't actually "cast" the spell
                    prop = QString( "%1 (Lvl %2)" ).arg( spell_name ).arg( spl.getLevel() );
                }
                else
                {
                    prop = QString( "%1 (Pwr %2)" ).arg( spell_name ).arg( power );
                }
            }
            break;
        }

        case DialogChooseColumns::HPRegen:
        {
            if (numeric)
                *numeric = true;

            if (i->getHPRegen() > 0)
                prop = QString( "+%1" ).arg( i->getHPRegen() );
            else if (i->getHPRegen() < 0)
                prop = QString( "%1" ).arg( i->getHPRegen() );
            break;
        }

        case DialogChooseColumns::StaminaRegen:
        {
            if (numeric)
                *numeric = true;

            if (i->getStaminaRegen() > 0)
                prop = QString( "+%1" ).arg( i->getStaminaRegen() );
            else if (i->getStaminaRegen() < 0)
                prop = QString( "%1" ).arg( i->getStaminaRegen() );
            break;
        }

        case DialogChooseColumns::SPRegen:
        {
            if (numeric)
                *numeric = true;

            if (i->getSPRegen() > 0)
                prop = QString( "+%1" ).arg( i->getSPRegen() );
            else if (i->getSPRegen() < 0)
                prop = QString( "%1" ).arg( i->getSPRegen() );
            break;
        }

        case DialogChooseColumns::AttribBonus:
        {
            int bonus;
            character::attribute a = i->getAttributeBonus( &bonus );

            if ((a != character::attribute::ATTRIBUTE_NONE) && (bonus != 0))
            {
                QString attribStr = ::getBaseStringTable()->getString( StringList::LISTPrimaryAttributes + static_cast<int>(a) );

                if (bonus > 0)
                {
                    prop = QString( "%1 +%2" ).arg( attribStr ).arg( bonus );
                }
                else
                {
                    prop = QString( "%1 %2" ).arg( attribStr ).arg( bonus );
                }
            }
            break;
        }

        case DialogChooseColumns::SkillBonus:
        {
            int bonus;
            character::skill sk = i->getSkillBonus( &bonus );

            if ((sk != character::skill::SKILL_NONE) && (bonus != 0))
            {
                QString skillStr = ::getBaseStringTable()->getString( StringList::LISTSkills + static_cast<int>(sk) );

                if (bonus > 0)
                {
                    prop = QString( "%1 +%2" ).arg( skillStr ).arg( bonus );
                }
                else
                {
                    prop = QString( "%1 %2" ).arg( skillStr ).arg( bonus );
                }
            }
            break;
        }

        case DialogChooseColumns::MagicResistances:
        {
            int f=0, w=0, a=0, e=0, m=0, d=0;

            if (i->getResistance(&f, &w, &a, &e, &m, &d))
            {
                QString list;

                // Possibly a problem for some languages. Expectation is the first 2 chars of all of these are ", "
                // so they can be jumped over for the first item in the list by the mid() call below
                if (f) list += QString( tr(", %1% vs Fire") ).arg(f);
                if (w) list += QString( tr(", %1% vs Water") ).arg(w);
                if (a) list += QString( tr(", %1% vs Air") ).arg(a);
                if (e) list += QString( tr(", %1% vs Earth") ).arg(e);
                if (m) list += QString( tr(", %1% vs Mental") ).arg(m);
                if (d) list += QString( tr(", %1% vs Divine") ).arg(d);

                prop = list.mid(2);
            }
            break;
        }

        case DialogChooseColumns::Required:
        {
            QString required = i->getRequiredAttribsString();
            QString required_s = i->getRequiredSkillsString();
            if (required.size() == 0)
            {
                required   = required_s;
                required_s = "";
            }
            /* NOT else if */
            if (required.size() > 0)
            {
                if (required_s.size() > 0)
                    required += ", " + required_s;

                prop = required;
            }
            break;
        }

        case DialogChooseColumns::AttackModes:
        {
            prop = i->getAttacksString();
            break;
        }

        case DialogChooseColumns::Special:
        {
            prop = i->getSpecialAttackString();
            if (i->getBonusSwings() != 0)
                prop += QString("; %1 bonus swings").arg( i->getBonusSwings() );
            if (i->getHPRegen() != 0)
                prop += QString("; %1 HP Regen").arg( i->getHPRegen() );
            if (i->getStaminaRegen() != 0)
                prop += QString("; %1 Stamina Regen").arg( i->getStaminaRegen() );
            if (i->getSPRegen() != 0)
                prop += QString("; %1 SP Regen").arg( i->getSPRegen() );

            int bonus;
            character::attribute a = i->getAttributeBonus( &bonus );

            if ((a != character::attribute::ATTRIBUTE_NONE) && (bonus != 0))
            {
                QString attribStr = ::getBaseStringTable()->getString( StringList::LISTPrimaryAttributes + static_cast<int>(a) );

                if (bonus > 0)
                {
                    prop += QString( "; %1 +%2" ).arg( attribStr ).arg( bonus );
                }
                else
                {
                    prop += QString( "; %1 %2" ).arg( attribStr ).arg( bonus );
                }
            }

            character::skill sk = i->getSkillBonus( &bonus );

            if ((sk != character::skill::SKILL_NONE) && (bonus != 0))
            {
                QString skillStr = ::getBaseStringTable()->getString( StringList::LISTSkills + static_cast<int>(sk) );

                if (bonus > 0)
                {
                    prop += QString( "; %1 +%2" ).arg( skillStr ).arg( bonus );
                }
                else
                {
                    prop += QString( "; %1 %2" ).arg( skillStr ).arg( bonus );
                }
            }
            if (prop.startsWith("; "))
                prop = prop.mid(2);
            break;
        }

        case DialogChooseColumns::Profs:
        {
            character::professions profs =  i->getUsableProfessions();

            prop  = (profs & character::profession::Fighter  ) ? "F" : "-";
            prop += (profs & character::profession::Lord     ) ? "L" : "-";
            prop += (profs & character::profession::Valkyrie ) ? "V" : "-";
            prop += (profs & character::profession::Ranger   ) ? "R" : "-";
            prop += (profs & character::profession::Samurai  ) ? "S" : "-";
            prop += (profs & character::profession::Ninja    ) ? "N" : "-";
            prop += (profs & character::profession::Monk     ) ? "m" : "-";
            prop += (profs & character::profession::Rogue    ) ? "r" : "-";
            prop += (profs & character::profession::Gadgeteer) ? "G" : "-";
            prop += (profs & character::profession::Bard     ) ? "b" : "-";
            prop += (profs & character::profession::Priest   ) ? "p" : "-";
            prop += (profs & character::profession::Alchemist) ? "A" : "-";
            prop += (profs & character::profession::Bishop   ) ? "B" : "-";
            prop += (profs & character::profession::Psionic  ) ? "P" : "-";
            prop += (profs & character::profession::Mage     ) ? "M" : "-";

            break;
        }

        case DialogChooseColumns::Races:
        {
            character::races races =  i->getUsableRaces();

            prop  = (races & character::race::Human    ) ? "H" : "-";
            prop += (races & character::race::Elf      ) ? "E" : "-";
            prop += (races & character::race::Dwarf    ) ? "D" : "-";
            prop += (races & character::race::Gnome    ) ? "G" : "-";
            prop += (races & character::race::Hobbit   ) ? "h" : "-";
            prop += (races & character::race::Faerie   ) ? "F" : "-";
            prop += (races & character::race::Lizardman) ? "L" : "-";
            prop += (races & character::race::Dracon   ) ? "d" : "-";
            prop += (races & character::race::Felpurr  ) ? "f" : "-";
            prop += (races & character::race::Rawulf   ) ? "R" : "-";
            prop += (races & character::race::Mook     ) ? "M" : "-";
            prop += (races & character::race::Trynnie  ) ? "t" : "-";
            prop += (races & character::race::TRang    ) ? "T" : "-";
            prop += (races & character::race::Umpani   ) ? "U" : "-";
            prop += (races & character::race::Rapax    ) ? "r" : "-";
            prop += (races & character::race::Android  ) ? "A" : "-";

            break;
        }

        case DialogChooseColumns::Genders:
        {
            if (i->getUsableGenders() == character::gender::Male)
                prop = "M-";
            else if (i->getUsableGenders() == character::gender::Female)
                prop = "-F";
            else
                prop = "MF";
            break;
        }
    }

    return prop;
}
