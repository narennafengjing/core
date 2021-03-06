/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPILERPLUGINS_CLANG_COMPAT_HXX
#define INCLUDED_COMPILERPLUGINS_CLANG_COMPAT_HXX

#include <cstddef>
#include <utility>

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/StringRef.h"

#include "config_clang.h"

// Compatibility wrapper to abstract over (trivial) changes in the Clang API:
namespace compat {

inline clang::SourceLocation getBeginLoc(clang::Decl const * decl) {
#if CLANG_VERSION >= 80000
    return decl->getBeginLoc();
#else
    return decl->getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::Decl const * decl) {
#if CLANG_VERSION >= 80000
    return decl->getEndLoc();
#else
    return decl->getLocEnd();
#endif
}

inline clang::SourceLocation getBeginLoc(clang::DeclarationNameInfo const & info) {
#if CLANG_VERSION >= 80000
    return info.getBeginLoc();
#else
    return info.getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::DeclarationNameInfo const & info) {
#if CLANG_VERSION >= 80000
    return info.getEndLoc();
#else
    return info.getLocEnd();
#endif
}

inline clang::SourceLocation getBeginLoc(clang::Stmt const * stmt) {
#if CLANG_VERSION >= 80000
    return stmt->getBeginLoc();
#else
    return stmt->getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::Stmt const * stmt) {
#if CLANG_VERSION >= 80000
    return stmt->getEndLoc();
#else
    return stmt->getLocEnd();
#endif
}

inline clang::SourceLocation getBeginLoc(clang::CXXBaseSpecifier const * spec) {
#if CLANG_VERSION >= 80000
    return spec->getBeginLoc();
#else
    return spec->getLocStart();
#endif
}

inline clang::SourceLocation getEndLoc(clang::CXXBaseSpecifier const * spec) {
#if CLANG_VERSION >= 80000
    return spec->getEndLoc();
#else
    return spec->getLocEnd();
#endif
}

inline std::pair<clang::SourceLocation, clang::SourceLocation> getImmediateExpansionRange(
    clang::SourceManager const & SM, clang::SourceLocation Loc)
{
#if CLANG_VERSION >= 70000
    auto const csr = SM.getImmediateExpansionRange(Loc);
    if (csr.isCharRange()) { /*TODO*/ }
    return {csr.getBegin(), csr.getEnd()};
#else
    return SM.getImmediateExpansionRange(Loc);
#endif
}

inline bool isPointWithin(
    clang::SourceManager const & SM, clang::SourceLocation Location, clang::SourceLocation Start,
    clang::SourceLocation End)
{
#if CLANG_VERSION >= 60000
    return SM.isPointWithin(Location, Start, End);
#else
    return
        Location == Start || Location == End
        || (SM.isBeforeInTranslationUnit(Start, Location)
            && SM.isBeforeInTranslationUnit(Location, End));
#endif
}

inline clang::Expr const * IgnoreImplicit(clang::Expr const * expr) {
#if CLANG_VERSION >= 80000
    return expr->IgnoreImplicit();
#else
    using namespace clang;
    // Copy from Clang's lib/AST/Stmt.cpp, including <https://reviews.llvm.org/D50666> "Fix
    // Stmt::ignoreImplicit":
    Stmt const *s = expr;

    Stmt const *lasts = nullptr;

    while (s != lasts) {
        lasts = s;

        if (auto *ewc = dyn_cast<ExprWithCleanups>(s))
            s = ewc->getSubExpr();

        if (auto *mte = dyn_cast<MaterializeTemporaryExpr>(s))
            s = mte->GetTemporaryExpr();

        if (auto *bte = dyn_cast<CXXBindTemporaryExpr>(s))
            s = bte->getSubExpr();

        if (auto *ice = dyn_cast<ImplicitCastExpr>(s))
            s = ice->getSubExpr();
    }

    return static_cast<Expr const *>(s);
#endif
}

inline bool CPlusPlus17(clang::LangOptions const & opts) {
#if CLANG_VERSION >= 60000
    return opts.CPlusPlus17;
#else
    return opts.CPlusPlus1z;
#endif
}

inline bool EvaluateAsInt(clang::Expr const * expr, llvm::APSInt& intRes, const clang::ASTContext& ctx) {
#if CLANG_VERSION >= 80000
    clang::Expr::EvalResult res;
    bool b = expr->EvaluateAsInt(res, ctx);
    if (b && res.Val.isInt())
        intRes = res.Val.getInt();
    return b;
#else
    return expr->EvaluateAsInt(intRes, ctx);
#endif
}

// Work around <http://reviews.llvm.org/D22128>:
//
// SfxErrorHandler::GetClassString (svtools/source/misc/ehdl.cxx):
//
//   ErrorResource_Impl aEr(aId, (sal_uInt16)lClassId);
//   if(aEr)
//   {
//       rStr = static_cast<ResString>(aEr).GetString();
//   }
//
// expr->dump():
//  CXXStaticCastExpr 0x2b74e8e657b8 'class ResString' static_cast<class ResString> <ConstructorConversion>
//  `-CXXBindTemporaryExpr 0x2b74e8e65798 'class ResString' (CXXTemporary 0x2b74e8e65790)
//    `-CXXConstructExpr 0x2b74e8e65758 'class ResString' 'void (class ResString &&) noexcept(false)' elidable
//      `-MaterializeTemporaryExpr 0x2b74e8e65740 'class ResString' xvalue
//        `-CXXBindTemporaryExpr 0x2b74e8e65720 'class ResString' (CXXTemporary 0x2b74e8e65718)
//          `-ImplicitCastExpr 0x2b74e8e65700 'class ResString' <UserDefinedConversion>
//            `-CXXMemberCallExpr 0x2b74e8e656d8 'class ResString'
//              `-MemberExpr 0x2b74e8e656a0 '<bound member function type>' .operator ResString 0x2b74e8dc1f00
//                `-DeclRefExpr 0x2b74e8e65648 'struct ErrorResource_Impl' lvalue Var 0x2b74e8e653b0 'aEr' 'struct ErrorResource_Impl'
// expr->getSubExprAsWritten()->dump():
//  MaterializeTemporaryExpr 0x2b74e8e65740 'class ResString' xvalue
//  `-CXXBindTemporaryExpr 0x2b74e8e65720 'class ResString' (CXXTemporary 0x2b74e8e65718)
//    `-ImplicitCastExpr 0x2b74e8e65700 'class ResString' <UserDefinedConversion>
//      `-CXXMemberCallExpr 0x2b74e8e656d8 'class ResString'
//        `-MemberExpr 0x2b74e8e656a0 '<bound member function type>' .operator ResString 0x2b74e8dc1f00
//          `-DeclRefExpr 0x2b74e8e65648 'struct ErrorResource_Impl' lvalue Var 0x2b74e8e653b0 'aEr' 'struct ErrorResource_Impl'
//
// Copies code from Clang's lib/AST/Expr.cpp:
namespace detail {
  inline clang::Expr *skipImplicitTemporary(clang::Expr *expr) {
    // Skip through reference binding to temporary.
    if (clang::MaterializeTemporaryExpr *Materialize
                                  = clang::dyn_cast<clang::MaterializeTemporaryExpr>(expr))
      expr = Materialize->GetTemporaryExpr();

    // Skip any temporary bindings; they're implicit.
    if (clang::CXXBindTemporaryExpr *Binder = clang::dyn_cast<clang::CXXBindTemporaryExpr>(expr))
      expr = Binder->getSubExpr();

    return expr;
  }
}
inline clang::Expr *getSubExprAsWritten(clang::CastExpr *This) {
  clang::Expr *SubExpr = nullptr;
  clang::CastExpr *E = This;
  do {
    SubExpr = detail::skipImplicitTemporary(E->getSubExpr());

    // Conversions by constructor and conversion functions have a
    // subexpression describing the call; strip it off.
    if (E->getCastKind() == clang::CK_ConstructorConversion)
      SubExpr =
        detail::skipImplicitTemporary(clang::cast<clang::CXXConstructExpr>(SubExpr)->getArg(0));
    else if (E->getCastKind() == clang::CK_UserDefinedConversion) {
      assert((clang::isa<clang::CXXMemberCallExpr>(SubExpr) ||
              clang::isa<clang::BlockExpr>(SubExpr)) &&
             "Unexpected SubExpr for CK_UserDefinedConversion.");
      if (clang::isa<clang::CXXMemberCallExpr>(SubExpr))
        SubExpr = clang::cast<clang::CXXMemberCallExpr>(SubExpr)->getImplicitObjectArgument();
    }

    // If the subexpression we're left with is an implicit cast, look
    // through that, too.
  } while ((E = clang::dyn_cast<clang::ImplicitCastExpr>(SubExpr)));

  return SubExpr;
}
inline const clang::Expr *getSubExprAsWritten(const clang::CastExpr *This) {
  return getSubExprAsWritten(const_cast<clang::CastExpr *>(This));
}

inline bool isExplicitSpecified(clang::CXXConstructorDecl const * decl) {
#if CLANG_VERSION >= 90000
    return decl->getExplicitSpecifier().isExplicit();
#else
    return decl->isExplicitSpecified();
#endif
}

inline bool isExplicitSpecified(clang::CXXConversionDecl const * decl) {
#if CLANG_VERSION >= 90000
    return decl->getExplicitSpecifier().isExplicit();
#else
    return decl->isExplicitSpecified();
#endif
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
