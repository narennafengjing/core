/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <TablesSingleDlg.hxx>
#include "DbAdminImpl.hxx"
#include "tablespage.hxx"
#include <dsitems.hxx>
#include <dbu_dlg.hxx>

namespace dbaui
{
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

    // OTableSubscriptionDialog
OTableSubscriptionDialog::OTableSubscriptionDialog(weld::Window* pParent
            ,const SfxItemSet* _pItems
            ,const Reference< XComponentContext >& _rxORB
            ,const css::uno::Any& _aDataSourceName)
    : SfxSingleTabDialogController(pParent, _pItems,
        "dbaccess/ui/tablesfilterdialog.ui", "TablesFilterDialog")
    , m_pImpl(new ODbDataSourceAdministrationHelper(_rxORB, m_xDialog.get(), pParent, this))
    , m_bStopExecution(false)
{
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet.reset(new SfxItemSet( *_pItems ));

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);
    SetInputSet(m_pOutSet.get());

    TabPageParent pPageParent(get_content_area(), this);
    auto pTabPage = VclPtrInstance<OTableSubscriptionPage>(pPageParent, *m_pOutSet, this);
    pTabPage->SetServiceFactory(_rxORB);
    SetTabPage(pTabPage);
}

OTableSubscriptionDialog::~OTableSubscriptionDialog()
{
}

short OTableSubscriptionDialog::run()
{
    short nRet = RET_CANCEL;
    if ( !m_bStopExecution )
    {
        nRet = SfxSingleTabDialogController::run();
        if ( nRet == RET_OK )
        {
            m_pOutSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*m_pOutSet);
        }
    }
    return nRet;
}

bool OTableSubscriptionDialog::getCurrentSettings(Sequence< PropertyValue >& _rDriverParams)
{
    return m_pImpl->getCurrentSettings(_rDriverParams);
}

void OTableSubscriptionDialog::successfullyConnected()
{
    m_pImpl->successfullyConnected();
}

void OTableSubscriptionDialog::clearPassword()
{
    m_pImpl->clearPassword();
}

OUString OTableSubscriptionDialog::getConnectionURL() const
{
    return m_pImpl->getConnectionURL();
}

Reference< XPropertySet > const & OTableSubscriptionDialog::getCurrentDataSource()
{
    return m_pImpl->getCurrentDataSource();
}

const SfxItemSet* OTableSubscriptionDialog::getOutputSet() const
{
    return m_pOutSet.get();
}

SfxItemSet* OTableSubscriptionDialog::getWriteOutputSet()
{
    return m_pOutSet.get();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
