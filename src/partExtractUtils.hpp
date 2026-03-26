#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <filesystem>
#include <sstream>
#include <ostream>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator> 
#include <stdexcept>

#include <GL/glew.h>
#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>


#include <CubismFramework.hpp>
#include "LAppDefine.hpp"
#include "LAppAllocator_Common.hpp"
#include "LAppTextureManager.hpp"
#include "LAppPal.hpp"
#include "CubismUserModelExtend.hpp"
#include "MouseActionManager.hpp"


//get the model path
std::vector<std::filesystem::path> GetModelFolders(const std::filesystem::path& resourcePath);


inline long long get_current_time() {
    auto now = std::chrono::system_clock::now();
    // Convert to duration since epoch and then to seconds
    auto duration_since_epoch = now.time_since_epoch();
    long long seconds = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch).count();
    return seconds;
}


inline size_t split_str(const std::string& txt, std::vector<std::string>& strs, char ch)
{
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while (pos != std::string::npos) {
        strs.push_back(txt.substr(initialPos, pos - initialPos));
        initialPos = pos + 1;

        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(
        txt.substr(initialPos, (std::min)(pos, txt.size()) - initialPos + 1)
    );

    return strs.size();
}


inline std::string sec2str(long long sec)
{
    std::string t;
    long long t_left = sec;
    long long h = t_left / 3600;
    if (h > 0)
    {
        t += std::to_string(h) + "h";
    }
    t_left -= h * 3600;
    
    long long m = t_left / 60;
    if (m > 0)
    {
        t += std::to_string(m) + "m";
    }
    t_left -= m * 60;

    t += std::to_string(t_left) + "s";

    return t;
}


bool cv_imwrite_lossless(const std::string& filename, const std::string& ext, cv::InputArray img);


bool SaveDrawableVertexInfo(
    Csm::CubismUserModel* userModel,
    int drawableIdx,
    const Csm::csmChar* drawableId,
    const std::filesystem::path& savep,
    YAML::Node& info
);


void CreateDirectoriesSafely(const std::filesystem::path& path);

std::string get_model_filep(const std::filesystem::path& src_dir, const std::string& fname);

bool str_endswith(const std::string& fullString,
              const std::string& ending);