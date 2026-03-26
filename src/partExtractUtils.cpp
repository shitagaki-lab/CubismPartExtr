#include "partExtractUtils.hpp"



std::vector<std::filesystem::path> GetModelFolders(const std::filesystem::path& resourcePath)
{
    std::vector<std::filesystem::path> folders;
    for (const auto& entry : std::filesystem::directory_iterator(resourcePath))
    {
        if (entry.is_directory())
        {
            folders.push_back(entry.path().filename());
        }
    }
    return folders;
}


// Function to check if a string ends with another string
bool str_endswith(const std::string& fullString,
              const std::string& ending)
{
    // Check if the ending string is longer than the full
    // string
    if (ending.size() > fullString.size())
        return false;

    // Compare the ending of the full string with the target
    // ending
    return fullString.compare(fullString.size()
                                  - ending.size(),
                              ending.size(), ending)
           == 0;
}



std::string get_model_filep(const std::filesystem::path& src_dir, const std::string& fname)
{
    std::string fileName = fname + ".model3.json";
    std::filesystem::path fp = src_dir / fileName;
    if (std::filesystem::exists(fp)){
        return fileName;
    }
    else {
        for (const auto& entry : std::filesystem::directory_iterator(src_dir)) {
            std::string fname = entry.path().filename();
            if (str_endswith(fname, ".model3.json"))
            {
                return entry.path().filename().string();
            }
        }
    }
    if (!std::filesystem::exists(fp)){
        throw std::invalid_argument("Can't find valid model file given src_dir: " + src_dir.string());
    }
    return fileName;
}



bool cv_imwrite_lossless(const std::string& filename, const std::string& ext, cv::InputArray img)
{
    
    if (ext == ".webp")
    {
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_WEBP_QUALITY);
        compression_params.push_back(101); // Quality 100 for lossless
        return cv::imwrite(filename + ext, img, compression_params);
    }

    return cv::imwrite(filename + ext, img);;
    return true;
}



// Helper function to save drawable vertex information to a text file
bool SaveDrawableVertexInfo(
    Csm::CubismUserModel* userModel,
    int drawableIdx,
    const Csm::csmChar* drawableId,
    const std::filesystem::path& savep,
    YAML::Node& info
)
{
    Csm::csmInt32 vertexIndexCount = userModel->GetModel()->GetDrawableVertexIndexCount(drawableIdx);
    Csm::csmInt32 vertexCount = userModel->GetModel()->GetDrawableVertexCount(drawableIdx);
    const Csm::csmUint16* vertexIndices = userModel->GetModel()->GetDrawableVertexIndices(drawableIdx);
    const Live2D::Cubism::Core::csmVector2* vertexPositions = userModel->GetModel()->GetDrawableVertexPositions(drawableIdx);
    const Live2D::Cubism::Core::csmVector2* vertexUvs = userModel->GetModel()->GetDrawableVertexUvs(drawableIdx);
    // outFile << "canvas_size: [" << canvasWidth << "," << canvasHeight << "]\n";

    info["drawable_index"] = drawableIdx;
    info["drawable_id"] = drawableId;
    info["vertex_indices"];
    for (size_t i = 0; i < vertexIndexCount; ++i) {
        info["vertex_indices"].push_back(vertexIndices[i]);
    }
    info["vertex_pos"];
    for (size_t i = 0; i < vertexCount; ++i) {
        info["vertex_pos"].push_back(vertexPositions[i].X);
        info["vertex_pos"].push_back(vertexPositions[i].Y);
    }


    // outFile << "transform_matrix: [";
    // auto mat = userModel->GetModelMatrix()->GetArray();
    // for (size_t i = 0; i < 16; ++i) {
    //     outFile << mat[i];
    //     if (i < 15) outFile << ",";
    // }
    // outFile << "]\n";

    // outFile << "vertex_pos: [";
    // for (size_t i = 0; i < vertexCount; ++i) {
    //     outFile << vertexPositions[i].X << "," << vertexPositions[i].Y;
    //     if (i < vertexCount - 1) outFile << ",";
    // }
    // outFile << "]\n";

    // outFile << "UVs=";
    // for (size_t i = 0; i < vertexCount; ++i) {
    //     outFile << "(" << vertexUvs[i].X << "," << vertexUvs[i].Y << ")";
    //     if (i < vertexCount - 1) outFile << ",";
    // }
    // outFile << "\n";

    // outFile.close();
    //std::filesystem::rename("tmp.txt", txtFilePath);
    return true;
}



void CreateDirectoriesSafely(const std::filesystem::path& path) {

        // Convert to absolute path
    std::filesystem::path absolutePath = std::filesystem::absolute(path);

    // Add the long path prefix if not already present
    // std::wstring longPath = absolutePath.wstring();
    // if (longPath.rfind(L"\\\\?\\", 0) != 0) {
    //     longPath = L"\\\\?\\" + longPath;
    // }

    // Create directories using the long path
    std::filesystem::create_directories(absolutePath);

    // try {
    //     // Convert to absolute path
    //     std::filesystem::path absolutePath = std::filesystem::absolute(path);

    //     // Add the long path prefix if not already present
    //     std::wstring longPath = absolutePath.wstring();
    //     if (longPath.rfind(L"\\\\?\\", 0) != 0) {
    //         longPath = L"\\\\?\\" + longPath;
    //     }

    //     // Create directories using the long path
    //     std::filesystem::create_directories(std::filesystem::path(longPath));
    // }
    // catch (const std::filesystem::filesystem_error& e) {
    //     // Handle the exception (e.g., log the error)
    //     return;
    // }
}
