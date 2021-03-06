// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/app_list/search/arc/arc_app_data_search_provider.h"

#include <memory>
#include <utility>

#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "chrome/browser/ui/app_list/search/arc/arc_app_data_search_result.h"
#include "components/arc/arc_bridge_service.h"
#include "components/arc/arc_service_manager.h"
#include "components/arc/connection_holder.h"

namespace app_list {

namespace {

// Checks if we're receiving a result with valid data from Android.
bool IsValidResult(const arc::mojom::AppDataResult& result) {
  if (result.launch_intent_uri.empty() || result.label.empty())
    return false;

  return true;
}

}  // namespace

ArcAppDataSearchProvider::ArcAppDataSearchProvider(
    int max_results,
    Profile* profile,
    AppListControllerDelegate* list_controller)
    : max_results_(max_results),
      profile_(profile),
      list_controller_(list_controller),
      weak_ptr_factory_(this) {}

ArcAppDataSearchProvider::~ArcAppDataSearchProvider() = default;

void ArcAppDataSearchProvider::Start(const base::string16& query) {
  arc::mojom::AppInstance* app_instance =
      arc::ArcServiceManager::Get()
          ? ARC_GET_INSTANCE_FOR_METHOD(
                arc::ArcServiceManager::Get()->arc_bridge_service()->app(),
                GetIcingGlobalQueryResults)
          : nullptr;

  if (!app_instance || query.empty()) {
    ClearResults();
    return;
  }

  weak_ptr_factory_.InvalidateWeakPtrs();
  app_instance->GetIcingGlobalQueryResults(
      base::UTF16ToUTF8(query), max_results_,
      base::BindOnce(&ArcAppDataSearchProvider::OnResults,
                     weak_ptr_factory_.GetWeakPtr(), base::TimeTicks::Now()));
}

void ArcAppDataSearchProvider::OnResults(
    base::TimeTicks query_start_time,
    arc::mojom::AppDataRequestState state,
    std::vector<arc::mojom::AppDataResultPtr> results) {
  if (state != arc::mojom::AppDataRequestState::REQUEST_SUCCESS) {
    DCHECK(results.empty());
    ClearResults();
    return;
  }

  SearchProvider::Results new_results;
  for (auto& result : results) {
    if (!IsValidResult(*result)) {
      ClearResults();
      return;
    }

    new_results.emplace_back(std::make_unique<ArcAppDataSearchResult>(
        std::move(result), profile_, list_controller_));
  }
  SwapResults(&new_results);
}

}  // namespace app_list
