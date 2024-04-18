plugins {
    alias(libs.plugins.androidApplication)
}

android {
    namespace = "com.example.lagrangianfluidsimulation"
    compileSdk = 34

    defaultConfig {
        applicationId = "com.example.lagrangianfluidsimulation"
        minSdk = 25
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
            abiFilters.addAll(listOf("x86_64"))
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
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
//    implementation(files("lib/netcdfAll-5.5.3.jar"))
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
}

//configurations.all {
//    resolutionStrategy {
//        // Exclude conflicting transitive dependencies globally
//        exclude(group = "com.google.guava", module = "listenablefuture")
//        exclude(group = "com.google.errorprone", module = "error_prone_annotations")
//    }
//}