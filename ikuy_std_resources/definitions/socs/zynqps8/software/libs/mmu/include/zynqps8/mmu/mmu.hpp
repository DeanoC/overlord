#pragma once

namespace Mmu {

enum class PageType {
	NormalCached,
	NormalUnCached,
	Device,
	Fault
};

struct Manager * Init(bool tablesFromHigh);

void Enable(struct Manager * mmu);

uintptr_all_t * InstallOrFetchL2(struct Manager * mmu, uintptr_all_t va);
uintptr_all_t * InstallOrFetchL3(struct Manager * mmu, uintptr_all_t va);

void UninstallL2(struct Manager * mmu, uintptr_all_t va);
void UninstallL3(struct Manager * mmu, uintptr_all_t va);

PageType GetPageType(struct Manager * mmu, uintptr_all_t va);
void SetPageTypeRange(struct Manager * mmu, uintptr_all_t va, size_t sizeInBytes, PageType pageType);

}
