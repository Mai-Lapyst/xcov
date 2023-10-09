pipeline {
    agent { label 'arch' }

    stages {
        stage('Prepare system') {
            steps {
                sh 'pacman --noconfirm -Syu'
                sh 'pacman --noconfirm -S base-devel curl gzip tar source-highlight boost boost-libs'
                sh './install_deps.sh'
            }
        }
        stage('Compile xcov') {
            environment {
                VERSION = sh(returnStdout: true, script: 'grep -Poe \'VERSION_STRING\\s*"\\K[^"]*\' ./src/args.cpp').trim()
            }
            steps {
                sh 'make release'
                sh 'PREFIX=./dist make install'
                sh "cd ./dist && tar -cvf ../xcov_${VERSION}.tar.xz *"
                archiveArtifacts artifacts: 'xcov_*.tar.xz', followSymlinks: false
            }
        }
    }
}