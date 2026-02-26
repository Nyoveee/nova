pipeline {
    agent any

    stages {
        stage('Build Debug') {
            steps {
                bat "msbuild Nova.sln /p:Configuration=Debug /p:Platform=x64 /m"
            }
        }
        stage('Build Release') {
            steps {
                bat "msbuild Nova.sln /p:Configuration=Release /p:Platform=x64 /m"
            }
        }
        stage('Build Installer') {
            steps {
                bat "msbuild Nova.sln /p:Configuration=Installer /p:Platform=x64 /m"
            }
        }
    }

    post {
        failure {
            echo 'Build failed!'
        }
        success {
            echo 'All configurations built successfully!'
        }
    }
}
