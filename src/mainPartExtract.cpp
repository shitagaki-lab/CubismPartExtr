/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include <iostream>
#include <fstream>

#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef ON_MACOS
#include <mach-o/dyld.h>
#include "libgen.h"
#endif

#include <CubismFramework.hpp>
#include <CubismModelSettingJson.hpp>
#include <Model/CubismUserModel.hpp>
#include <Physics/CubismPhysics.hpp>
#include <Rendering/OpenGL/CubismRenderer_OpenGLES2.hpp>
#include <Utils/CubismString.hpp>
#include "partExtractUtils.hpp"
#include <yaml-cpp/yaml.h>
/**
*@brief モデルデータのディレクトリ名
* このディレクトリ名と同名の.model3.jsonを読み込む
*/

static Csm::CubismUserModel* _userModel = nullptr; ///< ユーザーが実際に使用するモデル

Csm::csmFloat32 _userTimeSeconds; ///< デルタ時間の積算値[秒]
Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds; ///< モデルに設定されたまばたき機能用パラメータID
Csm::csmMap<Csm::csmString, Csm::ACubismMotion*> _motions; ///< 読み込まれているモーションのリスト
Csm::csmMap<Csm::csmString, Csm::ACubismMotion*> _expressions; ///< 読み込まれている表情のリスト

Csm::CubismPose* _pose;                      ///< ポーズ管理
Csm::CubismBreath* _breath;                    ///< 呼吸
Csm::CubismPhysics* _physics;                   ///< 物理演算
Csm::CubismEyeBlink* _eyeBlink;                  ///< 自動まばたき
Csm::CubismTargetPoint*_dragManager;               ///< マウスドラッグ
Csm::CubismModelMatrix* _modelMatrix;               ///< モデル行列
Csm::CubismMotionManager* _motionManager;             ///< モーション管理
Csm::CubismMotionManager* _expressionManager;         ///< 表情管理
Csm::CubismModelUserData* _modelUserData;             ///< ユーザデータ


Csm::csmFloat32 _dragX;                         ///< マウスドラッグのX位置
Csm::csmFloat32 _dragY;                         ///< マウスドラッグのY位置
Csm::csmFloat32 _accelerationX;                 ///< X軸方向の加速度
Csm::csmFloat32 _accelerationY;                 ///< Y軸方向の加速度
Csm::csmFloat32 _accelerationZ;                 ///< Z軸方向の加速度

static Csm::CubismFramework::Option _cubismOption; ///< CubismFrameworkに関するオプション
static LAppAllocator_Common _cubismAllocator; ///< メモリのアロケーター

static LAppTextureManager* _textureManager; ///< テクスチャの管理

static GLFWwindow* _window; ///< ウィンドウオブジェクト

int windowWidth, windowHeight; ///< ウィンドウサイズの保存

/**
* @brief Cubism SDKの初期化
*
* Cubism SDKの初期化処理を行う
*/
static void InitializeCubism()
{
    //setup cubism
    _cubismOption.LogFunction = LAppPal::PrintMessage;
    _cubismOption.LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;
    _cubismOption.LoadFileFunction = LAppPal::LoadFileAsBytes;
    _cubismOption.ReleaseBytesFunction = LAppPal::ReleaseBytes;
    Csm::CubismFramework::StartUp(&_cubismAllocator, &_cubismOption);

    //Initialize cubism
    Csm::CubismFramework::Initialize();
}

/**
* @brief アプリケーションの実行パスの設定
*
* アプリケーションの実行パスを確認し、パスを取得する
*/
void SetExecuteAbsolutePath()
{
#ifdef ON_MACOS
    char path[1024];
    uint32_t size = sizeof(path);
    _NSGetExecutablePath(path, &size);
    std::string _executeAbsolutePath = dirname(path);
    _executeAbsolutePath += "/";
    LAppPal::SetExecutableAbsolutePath(_executeAbsolutePath);
#endif
}

/**
* @brief システムの初期化
*
* 基盤システムの初期化処理を行う
*/
static bool InitializeSystem()
{
    LAppPal::PrintLogLn("START");
    SetExecuteAbsolutePath();

    // GLFWの初期化
    if (glfwInit() == GL_FALSE)
    {
        LAppPal::PrintLogLn("Can't initilize GLFW");
        return GL_FALSE;
    }

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // Windowの生成
    _window = glfwCreateWindow(LAppDefine::RenderTargetWidth, LAppDefine::RenderTargetHeight, "SIMPLE_SAMPLE", NULL, NULL);
    if (_window == NULL)
    {
        LAppPal::PrintLogLn("Can't create GLFW window.");

        glfwTerminate();
        return GL_FALSE;
    }

    // Windowのコンテキストをカレントに設定
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        LAppPal::PrintLogLn("Can't initilize glew.");

        glfwTerminate();
        return GL_FALSE;
    }


    //テクスチャサンプリング設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //透過設定
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // コールバック関数の登録
    glfwSetMouseButtonCallback(_window, EventHandler::OnMouseCallBack);
    glfwSetCursorPosCallback(_window, EventHandler::OnMouseCallBack);

    // ウィンドウサイズ記憶
    glfwGetWindowSize(_window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    // Cubism SDK の初期化
    InitializeCubism();

    // ドラッグ入力管理クラスの初期化
    MouseActionManager::GetInstance()->Initialize(windowWidth, windowHeight);

    // SetExecuteAbsolutePath();

    return GL_TRUE;
}



/**
* @brief 解放
*
* 解放の処理を行う
*/
void Release()
{
    // // レンダラの解放
    if (_userModel != nullptr)
    {
    _userModel->DeleteRenderer();
    // モデルデータの解放
    delete _userModel;
    _userModel = nullptr;
    }

    // テクスチャマネージャーの解放
    delete _textureManager;

    // Windowの削除
    glfwDestroyWindow(_window);

    // OpenGLの処理を終了
    glfwTerminate();

    // MouseActionManagerの解放
    MouseActionManager::ReleaseInstance();

    // Cubism SDK の解放
    Csm::CubismFramework::Dispose();
}



bool renderSaveFrame(const YAML::Node& config, const std::filesystem::path& savep, bool& skipped, std::vector<int>& crop_xyxy)
{
    skipped = false;
    std::string save_ext = ".png";
    if (config["save_ext"])
        save_ext = config["save_ext"].as<std::string>();

    bool crop_to_final = true;
    if (config["crop_to_final"])
        crop_to_final = config["crop_to_final"].as<bool>();

    // 画面の初期化
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearDepth(1.0);

    // モデルの更新及び描画
    static_cast<CubismUserModelExtend*>(_userModel)->ModelOnUpdate(_window);

    unsigned char* buffer = new unsigned char[ LAppDefine::RenderTargetWidth * LAppDefine::RenderTargetHeight * 4 ];
    glReadPixels( 0, 0, LAppDefine::RenderTargetWidth, LAppDefine::RenderTargetHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
    cv::Mat cv_img_frombuffer(LAppDefine::RenderTargetHeight, LAppDefine::RenderTargetWidth, CV_8UC4, buffer);
    
    cv::Mat cv_img;
    cv::flip(cv_img_frombuffer, cv_img, 0);
    delete[] buffer;
    cv::cvtColor(cv_img, cv_img, cv::COLOR_RGBA2BGRA);

    if (crop_to_final) {
        std::vector<cv::Mat> channels;
        cv::split(cv_img, channels);
        cv::Mat alpha_channel = channels[3]; // Alpha channel is typically the 4th channel (index 3)
        cv::Rect roi = cv::boundingRect(alpha_channel);
        if (roi.area() < 4)
        {
            skipped = true;
        }
        else
        {
            crop_xyxy.push_back(roi.x);
            crop_xyxy.push_back(roi.y);
            crop_xyxy.push_back(roi.x + roi.width);
            crop_xyxy.push_back(roi.y + roi.height);
            cv_img = cv_img(roi);
        }   
    }

    if (!skipped)
        return cv_imwrite_lossless(savep.string(), save_ext, cv_img);
    return true;
}


bool saveCurrentFrame(Csm::CubismUserModel* userModel, const YAML::Node& config, const std::filesystem::path& save_dir)
{
    bool save_drawables = true;
    if (config["save_drawables"])
        save_drawables = config["save_drawables"].as<bool>();

    auto GetPartFolderPath = [&](Csm::CubismModel* model, const Csm::csmInt32* partParentIds, Csm::csmInt32 partIndex) -> std::filesystem::path {
        std::vector<std::string> pathParts;
        while (partIndex >= 0) {
            const Csm::csmChar* partId = model->GetPartId(partIndex)->GetString().GetRawString();
            pathParts.push_back(partId);
            partIndex = partParentIds[partIndex];
        }
        std::filesystem::path fullPath;
        // 反轉順序，從 root 到 leaf
        for (auto it = pathParts.rbegin(); it != pathParts.rend(); ++it) {
            fullPath /= *it;
        }
        return fullPath;
        };

    auto cubismRenderer = _userModel->GetRenderer<Live2D::Cubism::Framework::Rendering::CubismRenderer_OpenGLES2>();

    bool skipped = true;
    std::vector<int> crop_xyxy{};
    CreateDirectoriesSafely(save_dir);
    std::filesystem::path final_savep = save_dir / "final";
    renderSaveFrame(config, final_savep, skipped, crop_xyxy);
    
    YAML::Node extr_info_root;
    extr_info_root.SetStyle(YAML::EmitterStyle::Flow);
    if (!crop_xyxy.empty())
    {   
        extr_info_root["crop_xyxy"] = crop_xyxy;
    }
    extr_info_root["canvas_size"].push_back(LAppDefine::RenderTargetWidth);
    extr_info_root["canvas_size"].push_back(LAppDefine::RenderTargetHeight);
    YAML::Node drawables;
    extr_info_root["drawables"] = drawables;

    Csm::CubismMatrix44 mat = cubismRenderer->GetMvpMatrix();
    Csm::csmFloat32* mvp_mat = mat.GetArray();

    extr_info_root["transform_matrix"];
    for (size_t i = 0; i < 16; ++i) {
        extr_info_root["transform_matrix"].push_back(mvp_mat[i]);
    }

    if (save_drawables)
    {
        const int drawableCount = userModel->GetModel()->GetDrawableCount();
        const int* renderOrder = userModel->GetModel()->GetDrawableRenderOrders();
        const int* drawableOrder = userModel->GetModel()->GetDrawableDrawOrders();
        const int* partParentIds = _userModel->GetModel()->GetPartParentPartIndices();

        for (int ii = 0; ii < drawableCount; ii++) {
            
            const int renderOrdervalue = renderOrder[ii];
            const int partIndex = userModel->GetModel()->GetDrawableParentPartIndex(ii);
            const int drawableOrdervalue = drawableOrder[ii];
            float opacity = userModel->GetModel()->GetDrawableOpacity(ii);
            cubismRenderer->SetRenderDrawableIdx(renderOrdervalue);

            if (opacity <= 0.005) continue;
            if (userModel->GetModel()->GetDrawableVertexCount(ii) <= 0) continue;

            const Csm::csmChar* drawableId = userModel->GetModel()->GetDrawableId(ii)->GetString().GetRawString();
            std::string drawableIdLower = drawableId;
            std::transform(drawableIdLower.begin(), drawableIdLower.end(), drawableIdLower.begin(), ::tolower);

            // Check if the lowercase drawable ID contains "hitarea"
            if (drawableIdLower.find("hitarea") != std::string::npos)
            {
                // Skip this drawable
                continue;
            }

            std::filesystem::path partFolderPath = save_dir / GetPartFolderPath(userModel->GetModel(), partParentIds, partIndex);
            CreateDirectoriesSafely(partFolderPath);
            
            std::filesystem::path saveName = partFolderPath / (std::to_string(renderOrdervalue) + "-" + drawableId + "-" + std::to_string(drawableOrdervalue));
            std::filesystem::path txt_savep = saveName.string() + ".yaml";
            crop_xyxy.clear();
            renderSaveFrame(config, saveName, skipped, crop_xyxy);
            
            if (!skipped) {
                YAML::Node drawable_info;
                SaveDrawableVertexInfo(_userModel, ii, drawableId, txt_savep, drawable_info);
                if (!crop_xyxy.empty())
                    drawable_info["crop_xyxy"] = crop_xyxy;
                drawables[renderOrdervalue] = drawable_info;
            }
        }
    }
    std::ofstream out_file(save_dir / "meta.yaml");
    YAML::Emitter emitter;
    emitter << extr_info_root;
    out_file << emitter.c_str();
    out_file.close();
    return true;
}





int processModel(const YAML::Node& config, const std::filesystem::path& root_dir, const std::filesystem::path& save_dir, std::filesystem::path& src_name)
{
    try{
        // src_name = std::filesystem::path("March 7th");
        const std::filesystem::path src_dir = root_dir / src_name;

        _userModel = new CubismUserModelExtend(src_name.string(), src_dir.string() + '/');
        std::string fileName = get_model_filep(src_dir, src_name);
        static_cast<CubismUserModelExtend*>(_userModel)->LoadAssets(fileName.c_str());

        // ユーザーモデルをMouseActionManagerへ渡す
        MouseActionManager::GetInstance()->SetUserModel(_userModel);

        int width, height;

        // ビューポートを現在のウィンドウサイズに設定
        glViewport(0, 0, windowWidth, windowHeight);

        // ウィンドウサイズ記憶
        glfwGetWindowSize(_window, &width, &height);
        if ((windowWidth != width || windowHeight != height) && width > 0 && height > 0)
        {
            //AppViewの初期化
            MouseActionManager::GetInstance()->ViewInitialize(width, height);
            // サイズを保存しておく
            windowWidth = width;
            windowHeight = height;

            // ビューポート変更
            glViewport(0, 0, width, height);
        }

        GLuint colorRenderbuffer;
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, LAppDefine::RenderTargetWidth, LAppDefine::RenderTargetHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

        saveCurrentFrame(_userModel, config, save_dir / src_name);

        // 時間更新
        // LAppPal::UpdateTime();

        glfwPollEvents();

    } catch (const YAML::ParserException& ex) {
        std::cerr << "YAML parsing error: " << ex.what() << std::endl;
        return -1;
    } 

    catch (const std::exception& e) {
        std::cerr << "failed to process model: " << root_dir / src_name << std::endl;
        std::cout << "Caught " << e.what() << std::endl;
// #ifndef RELEASE_VERSION
//         throw e;
// #endif
        return -1;
    }

    return 1;
}

int RunPartExtractLoop(int argc, char* argv[])
{
    // parse args & config
    const char* confgp = argv[1];
    YAML::Node config;
    try{
        config = YAML::LoadFile(confgp);
    } catch (const YAML::ParserException& ex) {
        std::cerr << "YAML parsing error: " << ex.what() << std::endl;
        return -1;
    }

    // load target src dirs, create saving dir
    std::filesystem::path root_dir = config["root_dir"].as<std::string>();
    std::filesystem::path save_dir = config["save_dir"].as<std::string>();
    std::filesystem::create_directories(save_dir);
    std::vector<std::filesystem::path> src_name_list = GetModelFolders(root_dir);

    InitializeSystem();

    size_t nfolders = src_name_list.size();
    int progress = 0;
    long long timestart = get_current_time();

    for (std::filesystem::path& src_name : src_name_list)
    {
        if (processModel(config, root_dir, save_dir, src_name)) {
            // // レンダラの解放
            if (_userModel != nullptr)
            {
            _userModel->DeleteRenderer();
            // モデルデータの解放
            delete _userModel;
            _userModel = nullptr;
            }
        }

        progress += 1;
        long long time_passed = get_current_time() - timestart;
        std::string time_passed_fmt = sec2str(time_passed);
        double iter_seconds = double(progress) / (time_passed);
        std::string left_seconds = sec2str((nfolders - progress) / iter_seconds);
        CubismLogInfo("progress %d/%d, time passed %s, eta %s", progress, nfolders, time_passed_fmt.c_str(), left_seconds.c_str());
    }

    Release();
    return 1;
}


int main(int argc, char* argv[])
{
    return RunPartExtractLoop(argc, argv);
}
