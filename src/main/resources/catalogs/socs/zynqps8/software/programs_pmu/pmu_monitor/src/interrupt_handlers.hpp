#pragma once

#include "interrupts.hpp"

void IPI0_Handler(Interrupts::Name irq_name);
void IPI3_Handler(Interrupts::Name irq_name);
void CorrectableECCErrors_Handler(Interrupts::Name irq_name);
void GPI0_Handler(Interrupts::Name irq_name);
void GPI1_Handler(Interrupts::Name irq_name);
void GPI2_Handler(Interrupts::Name irq_name);
void GPI3_Handler(Interrupts::Name irq_name);
void RTCAlarms_Handler(Interrupts::Name irq_name);
void RTCSeconds_Handler(Interrupts::Name irq_name);
