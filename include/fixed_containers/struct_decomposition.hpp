#pragma once

#include "fixed_containers/concepts.hpp"

#include <cstddef>

namespace fixed_containers::struct_decomposition
{
// Returning a `std::tuple` was used in the past and was very slow to compile, and for high
// enough numbers running out of memory before finishing.
//
// To generate the branches for for_each_field(), need macros, codegen, or manually creating all
// the branches. The solutions are explained in detail in:
// https://stackoverflow.com/questions/72275744/any-c20-alternative-tricks-to-p1061-structured-bindings-can-introduce-a-pack
//
// The macro solutions had big compilation times and for high enough numbers would even fail to
// compile. Using the codegen way instead. Use test/struct_decomposition_codegen.cpp to generate
// more branches if needed.

// clang-format off
// NOLINTBEGIN(readability-identifier-length, readability-identifier-naming, misc-confusable-identifiers)
// clang-format on
template <std::size_t C /*FIELD_COUNT*/, typename T, typename Func>
#if defined(FIXED_CONTAINERS_EXTENDED_STRUCT_DECOMPOSITION_1024) && \
    FIXED_CONTAINERS_EXTENDED_STRUCT_DECOMPOSITION_1024 == 1
// ifdef-ed to get the nice error message from the static_assert()
    requires(C <= 128)
#endif
constexpr auto to_parameter_pack(T& t, Func&& f)
{
    if constexpr (C == 0)
    {
    }
    // codegen-start
    // clang-format off
    else if constexpr(C <= 32) {
        if constexpr(C == 1) {
            auto& [mA] = t;
            return f(mA);
        }
        else if constexpr(C == 2) {
            auto& [mA,mB] = t;
            return f(mA,mB);
        }
        else if constexpr(C == 3) {
            auto& [mA,mB,mC] = t;
            return f(mA,mB,mC);
        }
        else if constexpr(C == 4) {
            auto& [mA,mB,mC,mD] = t;
            return f(mA,mB,mC,mD);
        }
        else if constexpr(C == 5) {
            auto& [mA,mB,mC,mD,mE] = t;
            return f(mA,mB,mC,mD,mE);
        }
        else if constexpr(C == 6) {
            auto& [mA,mB,mC,mD,mE,mF] = t;
            return f(mA,mB,mC,mD,mE,mF);
        }
        else if constexpr(C == 7) {
            auto& [mA,mB,mC,mD,mE,mF,mG] = t;
            return f(mA,mB,mC,mD,mE,mF,mG);
        }
        else if constexpr(C == 8) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH);
        }
        else if constexpr(C == 9) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI);
        }
        else if constexpr(C == 10) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ);
        }
        else if constexpr(C == 11) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK);
        }
        else if constexpr(C == 12) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL);
        }
        else if constexpr(C == 13) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM);
        }
        else if constexpr(C == 14) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN);
        }
        else if constexpr(C == 15) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO);
        }
        else if constexpr(C == 16) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP);
        }
        else if constexpr(C == 17) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ);
        }
        else if constexpr(C == 18) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR);
        }
        else if constexpr(C == 19) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS);
        }
        else if constexpr(C == 20) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT);
        }
        else if constexpr(C == 21) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU);
        }
        else if constexpr(C == 22) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV);
        }
        else if constexpr(C == 23) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW);
        }
        else if constexpr(C == 24) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX);
        }
        else if constexpr(C == 25) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY);
        }
        else if constexpr(C == 26) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ);
        }
        else if constexpr(C == 27) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma);
        }
        else if constexpr(C == 28) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb);
        }
        else if constexpr(C == 29) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc);
        }
        else if constexpr(C == 30) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md);
        }
        else if constexpr(C == 31) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me);
        }
        else if constexpr(C == 32) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf);
        }
    }
    else if constexpr(C <= 64) {
        if constexpr(C == 33) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg);
        }
        else if constexpr(C == 34) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh);
        }
        else if constexpr(C == 35) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi);
        }
        else if constexpr(C == 36) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj);
        }
        else if constexpr(C == 37) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk);
        }
        else if constexpr(C == 38) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml);
        }
        else if constexpr(C == 39) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm);
        }
        else if constexpr(C == 40) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn);
        }
        else if constexpr(C == 41) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo);
        }
        else if constexpr(C == 42) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp);
        }
        else if constexpr(C == 43) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq);
        }
        else if constexpr(C == 44) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr);
        }
        else if constexpr(C == 45) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms);
        }
        else if constexpr(C == 46) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt);
        }
        else if constexpr(C == 47) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu);
        }
        else if constexpr(C == 48) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv);
        }
        else if constexpr(C == 49) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw);
        }
        else if constexpr(C == 50) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx);
        }
        else if constexpr(C == 51) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my);
        }
        else if constexpr(C == 52) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz);
        }
        else if constexpr(C == 53) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0);
        }
        else if constexpr(C == 54) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1);
        }
        else if constexpr(C == 55) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2);
        }
        else if constexpr(C == 56) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3);
        }
        else if constexpr(C == 57) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4);
        }
        else if constexpr(C == 58) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5);
        }
        else if constexpr(C == 59) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6);
        }
        else if constexpr(C == 60) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7);
        }
        else if constexpr(C == 61) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8);
        }
        else if constexpr(C == 62) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9);
        }
        else if constexpr(C == 63) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA);
        }
        else if constexpr(C == 64) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB);
        }
    }
    else if constexpr(C <= 96) {
        if constexpr(C == 65) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC);
        }
        else if constexpr(C == 66) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD);
        }
        else if constexpr(C == 67) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE);
        }
        else if constexpr(C == 68) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF);
        }
        else if constexpr(C == 69) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG);
        }
        else if constexpr(C == 70) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH);
        }
        else if constexpr(C == 71) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI);
        }
        else if constexpr(C == 72) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ);
        }
        else if constexpr(C == 73) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK);
        }
        else if constexpr(C == 74) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL);
        }
        else if constexpr(C == 75) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM);
        }
        else if constexpr(C == 76) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN);
        }
        else if constexpr(C == 77) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO);
        }
        else if constexpr(C == 78) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP);
        }
        else if constexpr(C == 79) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ);
        }
        else if constexpr(C == 80) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR);
        }
        else if constexpr(C == 81) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS);
        }
        else if constexpr(C == 82) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT);
        }
        else if constexpr(C == 83) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU);
        }
        else if constexpr(C == 84) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV);
        }
        else if constexpr(C == 85) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW);
        }
        else if constexpr(C == 86) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX);
        }
        else if constexpr(C == 87) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY);
        }
        else if constexpr(C == 88) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ);
        }
        else if constexpr(C == 89) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa);
        }
        else if constexpr(C == 90) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb);
        }
        else if constexpr(C == 91) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc);
        }
        else if constexpr(C == 92) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd);
        }
        else if constexpr(C == 93) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe);
        }
        else if constexpr(C == 94) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf);
        }
        else if constexpr(C == 95) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg);
        }
        else if constexpr(C == 96) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh);
        }
    }
    else if constexpr(C <= 128) {
        if constexpr(C == 97) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi);
        }
        else if constexpr(C == 98) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj);
        }
        else if constexpr(C == 99) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk);
        }
        else if constexpr(C == 100) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl);
        }
        else if constexpr(C == 101) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm);
        }
        else if constexpr(C == 102) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn);
        }
        else if constexpr(C == 103) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo);
        }
        else if constexpr(C == 104) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp);
        }
        else if constexpr(C == 105) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq);
        }
        else if constexpr(C == 106) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr);
        }
        else if constexpr(C == 107) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs);
        }
        else if constexpr(C == 108) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt);
        }
        else if constexpr(C == 109) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu);
        }
        else if constexpr(C == 110) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv);
        }
        else if constexpr(C == 111) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw);
        }
        else if constexpr(C == 112) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx);
        }
        else if constexpr(C == 113) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy);
        }
        else if constexpr(C == 114) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz);
        }
        else if constexpr(C == 115) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0);
        }
        else if constexpr(C == 116) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1);
        }
        else if constexpr(C == 117) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2);
        }
        else if constexpr(C == 118) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3);
        }
        else if constexpr(C == 119) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4);
        }
        else if constexpr(C == 120) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5);
        }
        else if constexpr(C == 121) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6);
        }
        else if constexpr(C == 122) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7);
        }
        else if constexpr(C == 123) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8);
        }
        else if constexpr(C == 124) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9);
        }
        else if constexpr(C == 125) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA);
        }
        else if constexpr(C == 126) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA,mBB] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA,mBB);
        }
        else if constexpr(C == 127) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA,mBB,mBC] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA,mBB,mBC);
        }
        else if constexpr(C == 128) {
            auto& [mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA,mBB,mBC,mBD] = t;
            return f(mA,mB,mC,mD,mE,mF,mG,mH,mI,mJ,mK,mL,mM,mN,mO,mP,mQ,mR,mS,mT,mU,mV,mW,mX,mY,mZ,ma,mb,mc,md,me,mf,mg,mh,mi,mj,mk,ml,mm,mn,mo,mp,mq,mr,ms,mt,mu,mv,mw,mx,my,mz,m0,m1,m2,m3,m4,m5,m6,m7,m8,m9,mAA,mAB,mAC,mAD,mAE,mAF,mAG,mAH,mAI,mAJ,mAK,mAL,mAM,mAN,mAO,mAP,mAQ,mAR,mAS,mAT,mAU,mAV,mAW,mAX,mAY,mAZ,mAa,mAb,mAc,mAd,mAe,mAf,mAg,mAh,mAi,mAj,mAk,mAl,mAm,mAn,mAo,mAp,mAq,mAr,mAs,mAt,mAu,mAv,mAw,mAx,mAy,mAz,mA0,mA1,mA2,mA3,mA4,mA5,mA6,mA7,mA8,mA9,mBA,mBB,mBC,mBD);
        }
    }
    // clang-format on
    // codegen-end

    else
    {
        static_assert(AlwaysFalseV<T>,
                      "Too many fields for struct decomposition! You can define "
                      "FIXED_CONTAINERS_EXTENDED_STRUCT_DECOMPOSITION_1024 to extend this. Tools "
                      "like clang-tidy become slower with too large parameter count.");
    }
}

// clang-format off
// NOLINTEND(readability-identifier-length, readability-identifier-naming, misc-confusable-identifiers)
// clang-format on

}  // namespace fixed_containers::struct_decomposition

#if defined(FIXED_CONTAINERS_EXTENDED_STRUCT_DECOMPOSITION_1024) && \
    FIXED_CONTAINERS_EXTENDED_STRUCT_DECOMPOSITION_1024 == 1 &&     \
    (!defined(FIXED_CONTAINERS_CLANG_TIDY_RUNNING) || FIXED_CONTAINERS_CLANG_TIDY_RUNNING == 0)
#include "fixed_containers/struct_decomposition_129_to_512.hpp"
#include "fixed_containers/struct_decomposition_513_to_768.hpp"
#include "fixed_containers/struct_decomposition_769_to_1024.hpp"
#endif
