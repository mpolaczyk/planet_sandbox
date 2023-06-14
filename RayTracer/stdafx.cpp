#include "stdafx.h"
// https://stackoverflow.com/questions/6096384/how-to-fix-pch-file-missing-on-build

#include "app/asset.h"

// Define all globals here
namespace globals
{
  static asset_registry assets;

  asset_registry* get_asset_registry()
  {
    return &assets;
  }
};
