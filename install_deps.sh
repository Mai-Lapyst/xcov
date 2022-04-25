#!/bin/bash

# Dependency versions
nlohman_ver=v3.10.5
inja_ver=v3.3.0
bootstrap_ver=5.1.3

# Urls
nlohman_url=https://github.com/nlohmann/json/releases/download/$nlohman_ver/json.hpp
inja_url=https://raw.githubusercontent.com/pantor/inja/$inja_ver/single_include/inja/inja.hpp
bootstrap_url=https://cdn.jsdelivr.net/npm/bootstrap@$bootstrap_ver/dist/css/bootstrap.min.css

# Download libs
mkdir -p ./libs/nlohmann
pushd ./libs/nlohmann > /dev/null
curl -L -O $nlohman_url
popd > /dev/null

mkdir -p ./libs/inja
pushd ./libs/inja > /dev/null
curl -L -O $inja_url
popd > /dev/null

# Download bootstrap css
pushd ./xcov_data/themes/default > /dev/null
curl -L -O $bootstrap_url
popd > /dev/null