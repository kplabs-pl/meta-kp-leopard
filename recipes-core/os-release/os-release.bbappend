OS_RELEASE_FIELDS:append = " HOME_URL BUILD_ID"
OS_RELEASE_UNQUOTED_FIELDS:append = " BUILD_ID"


python __anonymous() {
    project_home_url = d.getVar('PROJECT_HOME_URL', True)
    if project_home_url in ['', None]:
        bb.warn('PROJECT_HOME_URL not set, consider setting it')
}

PROJECT_HOME_URL ?= ""
PROJECT_HOME_URL[doc] = "The URL of the project's home page"
HOME_URL = "${PROJECT_HOME_URL}"