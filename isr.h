#pragma once
#ifndef ISR_H
#define ISR_H

#if !defined(isr_off) || !defined(isr_on)
#define isr_off() noInterrupts()
#define isr_on() interrupts()
#endif

#endif