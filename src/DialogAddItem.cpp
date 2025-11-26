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

#include "DialogAddItem.h"
#include "SLFFile.h"
#include "STI.h"
#include "common.h"
#include "main.h"

#include <QApplication>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"
#include "WSpinBox.h"

#include <QDebug>

struct icon_init
{
    const char *filename;
    int         brightness;
};

struct icon_init type_icons[] =
{
    { "ITEMS/BLACKSWORD.STI",          64 },
    { "ITEMS/LANCE.STI",               64 },
    { "ITEMS/SHURIKEN2.STI",           64 },
    { "ITEMS/MUSKET.STI",              50 },
    { "ITEMS/ARROW.STI",               50 },
    { "ITEMS/MEDIUMSHIELD.STI",         0 },
    { "ITEMS/PLATETORSO.STI",          10 },
    { "ITEMS/FURLEGS.STI",             20 },
    { "ITEMS/BARBUTE.STI",             20 },
    { "ITEMS/COBALTINEPOWERGLOVE.STI", 64 },
    { "ITEMS/BOOTS.STI",               20 },
    { "ITEMS/DIAMONDRING.STI",         32 },
    { "ITEMS/QUEENSCLOAK.STI",         30 },
    { "ITEMS/LYRE.STI",                40 },
    { "ITEMS/DEMONINABOX.STI",         30 },
    { "ITEMS/SKULL.STI",               20 },
    { "ITEMS/BLUEPOTION.STI",          10 },
    { "ITEMS/STIX.STI",                10 },
    { "ITEMS/POWDERPOUCH.STI",         10 },
    { "ITEMS/BOOKTHREE.STI",           10 },
    { "ITEMS/SCROLL.STI",               0 },
    { "ITEMS/CHUM.STI",                10 },
    { "ITEMS/BROWNPOTION.STI",         64 },
    { "ITEMS/KEY.STI",                 10 },
    { "ITEMS/ROYALWRIT.STI",           64 },
    { "ITEMS/DEADFROG.STI",            20 }
};

typedef enum
{
    NO_ID,

    LBL_MULTI,
    VAL_MULTI,
    FRAME_TYPE,
    I_TYPE,
    VAL_TYPE,

    DDL_TOP,
    DDL_ROW2,
    DDL_ROW3,
    DDL_ROW4,
    DDL_ROW5,
    DDL_ROW6,
    DDL_ROW7,
    DDL_ROW8,
    DDL_ROW9,
    DDL_ROW10,
    DDL_ROW11,
    DDL_SCROLLLIST,
    DDL_BOTTOM,
    DDL_PREV,
    DDL_NEXT,

    ITEMS_SCROLLLIST,
    ITEMS_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;


DialogAddItem::DialogAddItem(int tag, QWidget *parent)
    : Dialog(parent),
    m_tag(tag),
    m_type(static_cast<item::type>(0))
{
    init();
    show();
}

// Edit an existing item
DialogAddItem::DialogAddItem(int tag, const item &existing_item, QWidget *parent)
    : Dialog(parent),
    m_tag(tag),
    m_type(static_cast<item::type>(0))
{
    init();

    // prepopulate the DDLs with the existing item and then deactivate those controls.

    m_type = existing_item.getType();

    updateList();

    if (WListWidget *w = qobject_cast<WListWidget *>(m_widgets[ ITEMS_SCROLLLIST ] ))
    {
        for (int k=0; k < w->count(); k++)
        {
            QListWidgetItem *wi = w->item(k);

            if (static_cast<item::type>( wi->data( Qt::UserRole ).toInt() ) == existing_item.getId())
            {
                w->setCurrentItem( wi );
                updateMulti( existing_item );
                break;
            }
        }

        w->setEnabled( false );
    }

    if (WImage *w = qobject_cast<WImage *>(m_widgets[ FRAME_TYPE ] ))
    {
        w->setEnabled( false );
    }
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ I_TYPE ] ))
    {
        w->setEnabled( false );
    }
    if (WLabel *w = qobject_cast<WLabel *>(m_widgets[ VAL_TYPE ] ))
    {
        w->setEnabled( false );
    }
    if (WButton *w = qobject_cast<WButton *>(m_widgets[ DDL_PREV ] ))
    {
        w->setEnabled( false );
    }
    if (WButton *w = qobject_cast<WButton *>(m_widgets[ DDL_NEXT ] ))
    {
        w->setEnabled( false );
    }

    show();
}

DialogAddItem::~DialogAddItem()
{
}

void DialogAddItem::init()
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    makeTypePixmaps();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { NO_ID,              QRect(  10,  32,  70,  12 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Type + StringList::APPEND_COLON ),  Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },
        { DDL_PREV,           QRect(  96,  25,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",               0, false, 1.0,  this ),  -1,  SLOT(prevType(bool)) },
        { FRAME_TYPE,         QRect( 122,  27,  -1,  -1 ),    new WImage(                                                                    this ),  -1,  SLOT(dropDownList(bool)) },
        { I_TYPE,             QRect( 125,  30,  -1,  -1 ),    new WImage(                                                                    this ),  -1,  SLOT(dropDownList(bool)) },
        { VAL_TYPE,           QRect( 145,  30, 120,  18 ),    new WLabel(    "", "Lucida Calligraphy",       Qt::AlignLeft,  9, QFont::Thin, this ),  -1,  SLOT(dropDownList(bool)) },
        { DDL_NEXT,           QRect( 278,  25,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",               5, false, 1.0,  this ),  -1,  SLOT(nextType(bool)) },
        { NO_ID,              QRect(  10,  75,  70,  12 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Item + StringList::APPEND_COLON ),  Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },

        { LBL_MULTI,          QRect(  10, 245,  70,  12 ),    new WLabel(    "",                            Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_MULTI,          QRect(  88, 245,  80,  12 ),    new WSpinBox(  1, 0, 255,                                                      this ),  -1,  NULL },
        { ITEMS_SCROLLBAR,    QRect( 354,  73,  15, 165 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },
        { ITEMS_SCROLLLIST,   QRect(  88,  72, 252, 166 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { NO_ID,              QRect( 312, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(addNewItem(bool)) },
        { NO_ID,              QRect( 344, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(close()) },

        { DDL_TOP,            QRect( 200,  50,  -1,  -1 ),    new WImage(    m_ddlTop,                                                       this ),  -1,  NULL },
        { DDL_ROW2,           QRect( 200,  72,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW3,           QRect( 200,  92,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW4,           QRect( 200, 112,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW5,           QRect( 200, 132,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW6,           QRect( 200, 152,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW7,           QRect( 200, 172,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW8,           QRect( 200, 192,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW9,           QRect( 200, 212,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW10,          QRect( 200, 232,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_ROW11,          QRect( 200, 252,  -1,  -1 ),    new WImage(    m_ddlMiddle,                                                    this ),  -1,  NULL },
        { DDL_BOTTOM,         QRect( 200, 272,  -1,  -1 ),    new WImage(    m_ddlBottom,                                                    this ),  -1,  NULL },
        { DDL_SCROLLLIST,     QRect( 202,  52, 152, 240 ),    new WListWidget(  "Lucida Calligraphy",                       9, QFont::Thin,  this ),  -1,  NULL },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // hide the Drop-down list until it is clicked on
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        ddl->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        ddl->clear();

        for (int k=0; k<=item::type::Other; k++)
        {
            QListWidgetItem *type = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTItemTypes + k ));
            type->setData( Qt::DecorationRole, m_typeIcon[ k ] );
            type->setData( Qt::UserRole, k );
            ddl->addItem( type );
        }
        connect( ddl, SIGNAL(itemClicked(QListWidgetItem *)),
                  this, SLOT(typeChanged(QListWidgetItem *)) );

        ddl->setMouseTracking(true);
        ddl->setStyleSheet("QListWidget { selection-color: #00df00; selection-background-color: transparent; } "
                           "*::item::hover { border-style: outset; border-width: 1px; border-color: beige; }");
    }
    for (int k=DDL_TOP; k<=DDL_BOTTOM; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }

    // Some manual mouseOver signal connection for the drop down list
    mouseOverDropDownList(false);
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ FRAME_TYPE ] ))
    {
        connect( w, SIGNAL(mouseOver(bool)), this, SLOT(mouseOverDropDownList(bool)) );
    }

    // The itemlist
    if (WListWidget *items = qobject_cast<WListWidget *>(m_widgets[ ITEMS_SCROLLLIST ] ))
    {
        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ ITEMS_SCROLLBAR ] ))
        {
            // QScrollAreas include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // scroll area and a short space away to the right

            items->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            items->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
            // return it to the same widget stack postion it was
            sb->stackUnder( m_widgets[ ITEMS_SCROLLLIST ] );
        }
        connect( items, SIGNAL(itemClicked(QListWidgetItem *)),
                  this, SLOT(itemChanged(QListWidgetItem *)) );
    }

    updateList();

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

void DialogAddItem::typeChanged(QListWidgetItem *now)
{
    if (now)
    {
        // Hide the drop down list
        for (int k=DDL_TOP; k<=DDL_BOTTOM; k++)
        {
            m_widgets[ k ]->setVisible( false );
        }

        // apply the selection
        m_type = static_cast<item::type>(now->data( Qt::UserRole ).toInt());

        updateList();
    }
}

void DialogAddItem::itemChanged(QListWidgetItem *now)
{
    if (now)
    {
        int item_id = now->data( Qt::UserRole ).toInt();

        item i( item_id );

        updateMulti( i );
    }
}

void DialogAddItem::mouseOverLabel(bool on)
{
    mouseOverDropDownList(on);
}

void DialogAddItem::mouseOverDropDownList(bool mouseover)
{
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ FRAME_TYPE ] ))
    {
        if (w->isEnabled())
        {
            if (mouseover)
            {
                w->setPixmap( m_ddlActive );
            }
            else
            {
                w->setPixmap( m_ddlInactive );
            }
        }
    }
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_TYPE ] ))
    {
        if (q->isEnabled())
        {
            if (mouseover)
                q->setStyleSheet("QLabel {color: #ffffff}"); // White
            else
                q->setStyleSheet("");
        }
    }
}

void DialogAddItem::updateList()
{
    if (WLabel *w = qobject_cast<WLabel *>(m_widgets[ VAL_TYPE ] ))
    {
        w->setText( ::getBaseStringTable()->getString( StringList::LISTItemTypes + static_cast<int>(m_type) ));
    }
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ I_TYPE ] ))
    {
        w->setPixmap( m_typeIcon[ static_cast<int>(m_type) ] );
    }

    // Repopulate the items list
    if (WListWidget *items = qobject_cast<WListWidget *>(m_widgets[ ITEMS_SCROLLLIST ] ))
    {
        items->clear();

        int num_items = dbHelper::getHelper()->getNumItems();
        for (int k=0; k<num_items; k++)
        {
            item   i( k );

            if (i.getType() == m_type)
            {
                QListWidgetItem *newItem = new QListWidgetItem( i.getName() );
                newItem->setData( Qt::UserRole, k );

                items->addItem( newItem );
            }
        }
        items->sortItems(Qt::AscendingOrder);
        items->setCurrentRow(0);
    }
}

void DialogAddItem::prevType(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);

        int t = static_cast<int>(m_type);

        t--;
        if (t < 0)
            t = item::type::Other;
        m_type = static_cast<item::type>(t);


        updateList();
    }
}

void DialogAddItem::nextType(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);

        int t = static_cast<int>(m_type);

        t++;
        if (t > item::type::Other)
            t = 0;
        m_type = static_cast<item::type>(t);

        updateList();
    }
}

void DialogAddItem::dropDownList(bool down)
{
    if (down)
    {
        bool toggle = m_widgets[ DDL_TOP ]->isVisible();

        for (int k=DDL_TOP; k<=DDL_BOTTOM; k++)
        {
            m_widgets[ k ]->setVisible( !toggle );
        }
    }
}

void DialogAddItem::updateMulti( const item &i )
{
    WLabel   *w = qobject_cast<WLabel *>(m_widgets[ LBL_MULTI ]);
    WSpinBox *v = qobject_cast<WSpinBox *>(m_widgets[ VAL_MULTI ]);

    if (w && v)
    {
        v->setVisible( true );

        if (i.isStackable())
        {
            w->setText( ::getBaseStringTable()->getString( StringList::Quantity + StringList::APPEND_COLON ) );
            v->setRange( 0, 255 );
            if (i.getCount() == 0)
            {
                v->setValueEx( 1, 1 );
            }
            else
            {
                v->setValueEx( i.getCount(), i.getCount() );
            }
        }
        else if (i.hasCharges())
        {
            w->setText( ::getBaseStringTable()->getString( StringList::Charges + StringList::APPEND_COLON ) );
            v->setRange( 0, i.getMaxCharges() );
            if (i.getCharges() == 0)
            {
                v->setValueEx( i.getMaxCharges(), i.getMaxCharges() );
            }
            else
            {
                v->setValueEx( i.getCharges(), i.getCharges() );
            }
        }
        else if (i.hasUses())
        {
            w->setText( ::getBaseStringTable()->getString( StringList::Uses + StringList::APPEND_COLON ) );
            v->setRange( 0, i.getMaxCharges() );
            if (i.getCharges() == 0)
            {
                v->setValueEx( i.getMaxCharges(), i.getMaxCharges() );
            }
            else
            {
                v->setValueEx( i.getCharges(), i.getCharges() );
            }
        }
        else if (i.hasShots())
        {
            w->setText( ::getBaseStringTable()->getString( StringList::Shots + StringList::APPEND_COLON ) );
            v->setRange( 0, i.getMaxCharges() );
            if (i.getCharges() == 0)
            {
                v->setValueEx( i.getMaxCharges(), i.getMaxCharges() );
            }
            else
            {
                v->setValueEx( i.getCharges(), i.getCharges() );
            }
        }
        else
        {
            w->setText( "" );
            v->setVisible( false );
        }
    }
}

QPixmap DialogAddItem::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg,     bgPtn;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_ITEMINFO.STI", 0 );
    bgPtn = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( bgImg.width(), bgImg.height() + 20 ) );

    QPainter p;

    p.begin( &customImage );
    // First make it a little taller, by 20 rows and squeeze the hole for
    // the scrollarea and the buttonbox over 20 pixels to the right
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(), 242 );       // Top
    p.drawPixmap(  26,  69, bgImg,   6,  69, 66, 171 );                      // Shrunk scrollpane
    p.drawPixmap(   0, 177, bgImg,   0, 157,  6, bgImg.height() - 157 ); // LHS bevel
    p.drawPixmap( 378, 177, bgImg, 378, 157, bgImg.width() - 378, bgImg.height() - 157 ); // RHS bevel
    p.drawPixmap(   0, 262, bgImg,   0, 242, bgImg.width(), bgImg.height() - 242 ); // Bottom
    p.drawPixmap(  26, 262, bgImg,   6, 242, 66, bgImg.height() - 248 ); // Shrunk buttonbox
    // Now fill in the slot where the professions/races went, the slot for the icon, and the
    // empty space we just left in the middle.
    p.drawPixmap(   6,   6, bgPtn,   0,   0, 140,  64 ); // item slot
    p.drawPixmap(   6,   6, bgPtn,   0,   0,  82, 242 ); // most of the professions slot
    p.drawPixmap(   6,  bgImg.height() - bgPtn.height() + 124, bgPtn,   0, 110,  79, bgPtn.height() - 110);
    p.drawPixmap(   6, 240, bgPtn,   0, 190,  bgPtn.width(), 24 );       // LHS of the hole we made
    p.drawPixmap( 178, 240, bgPtn,   bgPtn.width()-200, 190,  200, 24 ); // RHS of the hole we made
    p.end();

    return customImage;
}

void DialogAddItem::makeTypePixmaps()
{
    // images used in the types list
    for (unsigned int j=0; j<sizeof(type_icons)/sizeof(type_icons[0]); j++)
    {
        SLFFile ic( type_icons[j].filename );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array;

            ic.readAll( array );
            STI c( array );

            QImage  im = c.getImage( 0 );

            // All item pixmaps are expected to be in ARGB32 - it'd
            // only be if a mod changed them
            //if (im.format() != QImage::Format_RGB32 )
            //    im = im.convertToFormat( QImage::Format_RGB32 );

            uchar *data = im.bits();
            for (int k=0; k < im.sizeInBytes(); k+=4)
            {
                int b = data[k+0] + type_icons[j].brightness;
                int g = data[k+1] + type_icons[j].brightness;
                int r = data[k+2] + type_icons[j].brightness;

                data[k+0] = ((b <= 255) ? b : 255);
                data[k+1] = ((g <= 255) ? g : 255);
                data[k+2] = ((r <= 255) ? r : 255);
            }
            m_typeIcon[j] = QPixmap::fromImage( im.scaledToHeight( 14 * m_scale, Qt::SmoothTransformation ) );

            ic.close();
        }
        else
        {
            m_typeIcon[j] = QPixmap();
        }
    }

    // Wider versions of the pixmaps provided by Wizardry
    SLFFile imgs( "CHAR GENERATION/CG_PROFESSION.STI" );
    if (imgs.open(QFile::ReadOnly))
    {
        QByteArray array;

        imgs.readAll( array );
        STI sti_imgs( array );

        m_ddlInactive = makeWider( sti_imgs.getImage( 1 ), sti_imgs.getWidth( 1 ) + 30 );
        m_ddlActive   = makeWider( sti_imgs.getImage( 2 ), sti_imgs.getWidth( 2 ) + 30 );
        m_ddlTop      = makeWider( sti_imgs.getImage( 4 ), sti_imgs.getWidth( 4 ) + 30 );
        m_ddlMiddle   = makeWider( sti_imgs.getImage( 5 ), sti_imgs.getWidth( 5 ) + 30 );
        m_ddlBottom   = makeWider( sti_imgs.getImage( 6 ), sti_imgs.getWidth( 6 ) + 30 );

        imgs.close();
    }
}

QPixmap DialogAddItem::makeWider( QImage im, int width )
{
    QPainter p;
    QPixmap  in = QPixmap::fromImage( im );
    QPixmap  out( width, in.height() );

    out.fill( Qt::red );
    p.begin(&out);
    p.drawPixmap( 0, 0, in );
    p.drawPixmap( width - in.width() + 6, 0, in, 6, 0, in.width() - 6, in.height() );
    p.end();

    return out;
}

void DialogAddItem::addNewItem(bool)
{
    if (WListWidget *items = qobject_cast<WListWidget *>(m_widgets[ ITEMS_SCROLLLIST ] ))
    {
        if (QListWidgetItem *it = items->currentItem())
        {
            int item_id = it->data( Qt::UserRole ).toInt();
;
            item i( item_id );

            if (WSpinBox *v = qobject_cast<WSpinBox *>(m_widgets[ VAL_MULTI ]))
            {
                if (i.isStackable())
                {
                    i.setCount( v->value() );
                }
                else if (i.hasCharges() ||
                         i.hasUses() ||
                         i.hasShots())
                {
                    i.setCharges( v->value() );
                }
            }
            emit itemAdded(m_tag, i);
        }
    }

    close();
}
