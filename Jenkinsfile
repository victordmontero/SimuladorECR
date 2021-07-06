pipeline {
	agent any
	stages {
		stage('Prerequisites') {
			steps {
				sh 'echo "Fetching premake5..."'
				sh 'wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha16/premake-5.0.0-alpha16-linux.tar.gz'
				sh 'tar -zxvf premake-5.0.0-alpha16-linux.tar.gz'
				sh 'chmod +x premake5'
				sh 'echo "Installing wxWidget..."'
				sh 'apt-get update'
				sh 'apt-get install wx3.0-headers libwxgtk3.0-* make build-essential -y'
			}
		}
		stage('Build') {
			steps {
				sh 'echo "Generating makefiles..."'
				sh './premake5 gmake2'
				sh 'cd proj_gmake2'
				sh 'make config=debug_linux'
				sh 'cd ..'
				archiveArtifacts artifacts: 'bin/Debug/*', fingerprint: true
			}
		}
	}
}