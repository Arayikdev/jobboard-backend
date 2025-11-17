export const categories = ["Frontend", "Backend", "DevOps"];
export const languages = ["JS", "Python", "Java", "C#"];
export const seniorityLevels = ["Intern", "Junior", "Mid", "Senior"];
export const skillsMap = {
  JS: {
    Frontend: ["React", "Redux", "Vue", "Next.js"],
    Backend: ["Node", "Express", "NestJS"],
    DevOps: ["Docker", "CI/CD"]
  },
  Python: {
    Backend: ["Django", "Flask", "FastAPI"],
    DevOps: ["Docker", "Kubernetes"]
  }
};

export const countsMap = {
  seniority: { Intern: 5, Junior: 12, Mid: 8, Senior: 3 },
  skills: { React: 10, Redux: 8, Node: 6, Django: 4 },
  category: { "FrontendJS": 10, "BackendJS": 6, "BackendPython": 4 },
};
