plugins {
    alias(libs.plugins.androidApplication)
}

android {
    namespace = "com.rug.lagrangianfluidsimulation"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.rug.lagrangianfluidsimulation"
        minSdk = 29
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
            }
        }

        ndk {
            abiFilters.addAll(listOf("arm64-v8a"))
        }
    }

    tasks.withType<JavaCompile> {
        options.compilerArgs.add("-Xlint:deprecation")
    }

    signingConfigs {
        create("release") {
            keyAlias = "key0"
            keyPassword = "Tomasko5"
            storeFile = file("../../keys/my-release-key.jks")
            storePassword = "Tomasko5"
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
            signingConfig = signingConfigs.getByName("release")
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
    buildFeatures {
        viewBinding = true
    }

    sourceSets {
        getByName("main") {
            resources.srcDirs("src/main/assets")
            resources.srcDirs("src/main/test_data")
            jniLibs.srcDirs("src/main/jniLibs")
        }
    }
}

dependencies {

    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.constraintlayout)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
    androidTestImplementation(libs.mockito.android)
    androidTestImplementation(libs.rules)
}

configurations.all {
    exclude(group = "androidx.emoji2", module = "emoji2")
    exclude(group = "androidx.emoji2", module = "emoji2-views-helper")
}
